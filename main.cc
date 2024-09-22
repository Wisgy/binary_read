#include "binary_arch.hh"
#include "state_machine.hh"
#include "workflow.hh"
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <cassert>
#include <cctype>
#include <iostream>
#include <string>
#include <sys/types.h>

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

void process_input_binary(const std::string &command,
                          const std::string &output_file) {
  // produce binary information
  TempFile temp_file;
  std::string full_command = command + " > " + temp_file.get_filename();
  std::system(full_command.c_str());

  // get binary information
  std::ifstream infile(temp_file.get_filename());
  if (!infile.is_open()) {
    throw std::runtime_error("Failed to open temporary file");
  }

  // process binary information
  ObjectFile *obj = new ObjectFile();
  std::string line;
  StateMachine sm;
  while (std::getline(infile, line)) {
    Workflow::run(line, obj, sm);
  }

  // output process result
  if (output_file.empty()) {
    obj->print(std::cout);
  } else {
    std::ofstream outfile(output_file);
    if (!outfile.is_open()) {
      throw std::runtime_error("Failed to open output file");
    }
    obj->print(outfile);
    outfile.close();
  }

  infile.close();
}

int main(int argc, char *argv[]) {
  try {
    // Define and parse the program options
    std::string input_file{};
    std::string output_file{};
    po::options_description configs("Analyze binary executable");
    configs.add_options()("help,h", "produce help message")(
        "input,i", po::value<std::string>(&input_file)->required(),
        "specify the input binary exectuable")(
        "output,o", po::value<std::string>(&output_file),
        "specify the anlysis output file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, configs), vm);

    // Handle options
    if (vm.count("help")) {
      std::cout << configs << "\n";
      return 1;
    }

    po::notify(vm);

    // Execute objdump command
    std::string objdump_command = "objdump -d " + input_file;
    process_input_binary(objdump_command, output_file);

  } catch (std::exception &e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "Exception of unknown type!\n";
  }

  return 0;
}