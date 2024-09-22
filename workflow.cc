#include "workflow.hh"
#include "sstream"

using sm = StateMachine;

namespace Workflow {
std::pair<sm::event, TextInfo> parse_text(std::string &text) {
  sm::event e = sm::event::NONE;
  TextInfo ti;
  if (text.find("Disassembly of section") != std::string::npos) {
    e = sm::event::NEWSECT;
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
      e = sm::event::PAD;
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
        e = sm::event::BRANCH;
      } else if (ti.name.compare(0, 3, "nop") == 0) {
        e = sm::event::PAD;
      } else {
        e = sm::event::INST;
      }
    }
  } else if (text.size() == 0) {
    e = sm::event::NEWLINE;
  } else if (text[0] >= '0' && text[0] <= '9') {
    e = sm::event::NEWFUNC;
    auto pos = text.find_first_of('<') + 1;
    auto num = text.find_first_of('>') - pos;
    ti.name = text.substr(pos, num);
    ti.address =
        std::stoull(text.substr(0, text.find_first_of(' ')), nullptr, 16);
  } else if (text[0] == '/') {
    e = sm::event::PATH;
  } else {
    throw std::runtime_error("Unknown text");
  }
  return std::make_pair(e, ti);
}

void handle_actions(std::vector<sm::action> &actions, ObjectFile *obj,
                    TextInfo &info) {
  static Section *sect = nullptr;
  static Function *func = nullptr;
  static BasicBlock *bb = nullptr;

  for (auto a : actions) {
    switch (a) {
    case sm::action::CRT_SECT:
      if (obj == nullptr) {
        throw std::runtime_error(
            "Logic Vulnerablity: Object file is not created");
      }
      sect = obj->create_section(info.name);
      break;
    case sm::action::CRT_FUNC:
      if (sect == nullptr) {
        throw std::runtime_error("Logic Vulnerablity: Section is not created");
      }
      func = sect->create_function(info.name, info.address);
      break;
    case sm::action::CRT_BB:
      if (func == nullptr) {
        throw std::runtime_error("Logic Vulnerablity: Function is not created");
      }
      bb = func->create_basic_block(info.address - func->get_start_address());
      break;
    case sm::action::CRT_INST:
      if (bb == nullptr) {
        throw std::runtime_error(
            "Logic Vulnerablity: Basic block is not created");
      }
      bb->create_instruction(info.address, info.code);
      break;
    case sm::action::CRT_PAD:
      if (sect == nullptr) {
        throw std::runtime_error("Logic Vulnerablity: Section is not created");
      }
      func->create_padding(info.address - func->get_start_address());
      break;
    }
  }
}

void run(std::string &text, ObjectFile *obj, StateMachine &sm) {
  auto info = parse_text(text);// parse input text to get event and text info
  auto actions = sm.get_next_action(info.first);// get next action based on event
  handle_actions(actions, obj, info.second);// handle the actions to build binary architecture
}
} // namespace Workflow