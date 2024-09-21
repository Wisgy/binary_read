#include "binary_arch.hh"
#include "state_machine.hh"
#include <boost/program_options.hpp>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <utility>

namespace po = boost::program_options;

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdexcept>

class TempFile {
public:
  TempFile() {
    char temp_name[] = "/tmp/objdump_output_XXXXXX";
    fd = mkstemp(temp_name);
    if (fd == -1) {
      throw std::runtime_error("Failed to create temporary file");
    }
    filename = temp_name;
  }

  ~TempFile() {
    if (fd != -1) {
      close(fd);
      std::remove(filename.c_str());
    }
  }

  const std::string &get_filename() const { return filename; }

private:
  int fd;
  std::string filename;
};

struct TextInfo {
  std::string name;
  std::string code;
  uint64_t address;
};
std::pair<event, TextInfo> parse_text(std::string &text) {
  event e = event::NONE;
  TextInfo ti;
  if (text.find("Disassembly of section") != std::string::npos) {
    e = event::NEWSECT;
    auto pos = text.find_first_of('.');
    auto num = text.find_first_of(':') - pos;
    ti.name = text.substr(pos, num);
  } else if (std::isspace(text[0])) {
    std::istringstream ss(text);
    std::string temp;
    // Parse address
    ss >> std::hex >> ti.address;
    ss.ignore(256, ':');
    // Parse instruction code
    ss >> ti.code;
    ss.ignore();
    while (not isspace(int(ss.peek())) and not ss.eof()) {
      std::string byte;
      ss >> byte;
      ti.code += byte;
      ss.ignore();
    }
    if (ss.eof()) {
      e = event::PAD;
      ti.name = "";
    } else {
      std::getline(ss, temp);
      // Trim comment
      size_t commentPos = temp.find('#');
      if (commentPos != std::string::npos) {
        temp = temp.substr(0, commentPos);
      }
      // Trim leading and trailing whitespaces
      auto start = temp.begin();
      auto end = temp.end();
      while (start != end && std::isspace(*start)) {
        ++start;
      }
      while (start != end && std::isspace(*(end - 1))) {
        --end;
      }
      ti.name = std::string(start, end);
      // Check properties of instruction
      if (ti.name[0] == 'j' || ti.name.compare(0, 4, "loop") == 0 ||
          ti.name.compare(0, 3, "ret") == 0) {
        e = event::BRANCH;
      } else if (ti.name.compare(0, 3, "nop") == 0) {
        e = event::PAD;
      } else {
        e = event::INST;
      }
    }
  } else if (text.size() == 0) {
    e = event::NEWLINE;
  } else if (text[0] >= '0' && text[0] <= '9') {
    e = event::NEWFUNC;
    auto pos = text.find_first_of('<') + 1;
    auto num = text.find_first_of('>') - pos;
    ti.name = text.substr(pos, num);
    ti.address =
        std::stoull(text.substr(0, text.find_first_of(' ')), nullptr, 16);
  } else if (text[0] == '/') {
    e = event::PATH;
  } else {
    throw std::runtime_error("Unknown text");
  }
  return std::make_pair(e, ti);
}

void pattern_match(std::ifstream &infile) {
  ObjectFile *obj = new ObjectFile();
  Section *sect = nullptr;
  Function *func = nullptr;
  BasicBlock *bb = nullptr;
  std::string line;
  state s = state::OBJECT;
  int times = 0;
  while (std::getline(infile, line)) {
    std::cout << times++ << std::endl;
    if (times == 336) {
      std::cout << "here" << std::endl;
    }
    auto info = parse_text(line);
    auto actions = transition(s, info.first);
    for (auto a : actions) {
      switch (a) {
      case act::CRT_SECT:
        if (obj == nullptr) {
          throw std::runtime_error(
              "Logic Vulnerablity: Object file is not created");
        }
        sect = obj->create_section(info.second.name);
        break;
      case act::CRT_FUNC:
        if (sect == nullptr) {
          throw std::runtime_error(
              "Logic Vulnerablity: Section is not created");
        }
        func = sect->create_function(info.second.name, info.second.address);
        break;
      case act::CRT_BB:
        if (func == nullptr) {
          throw std::runtime_error(
              "Logic Vulnerablity: Function is not created");
        }
        bb = func->create_basic_block(info.second.address -
                                      func->get_start_address());
        break;
      case act::CRT_INST:
        if (bb == nullptr) {
          throw std::runtime_error(
              "Logic Vulnerablity: Basic block is not created");
        }
        bb->create_instruction(info.second.address, info.second.code);
        break;
      case act::CRT_PAD:
        if (sect == nullptr) {
          throw std::runtime_error(
              "Logic Vulnerablity: Section is not created");
        }
        func->create_padding(info.second.address, info.second.code);
        break;
      }
    }
  }
  obj->print(std::cout);
}

void execute_and_parse_objdump(const std::string &command) {
  TempFile temp_file;
  std::string full_command = command + " > " + temp_file.get_filename();
  std::system(full_command.c_str());

  std::ifstream infile(temp_file.get_filename());
  if (!infile.is_open()) {
    throw std::runtime_error("Failed to open temporary file");
  }

  pattern_match(infile);

  infile.close();
}

int main(int argc, char *argv[]) {
  try {
    // Define and parse the program options
    std::string input_file;
    po::options_description configs("Allowed options");
    configs.add_options()("help,h", "produce help message")(
        "input,i", po::value<std::string>(&input_file)->required(),
        "set input file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, configs), vm);
    po::notify(vm);

    // Handle options
    if (vm.count("help")) {
      std::cout << configs << "\n";
      return 1;
    }

    // Execute objdump command
    std::string objdump_command = "objdump -d " + input_file;
    execute_and_parse_objdump(objdump_command);

  } catch (std::exception &e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!\n";
  }

  return 0;
}