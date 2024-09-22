#pragma once
#include "binary_arch.hh"
#include "state_machine.hh"
#include <cstdint>
#include <string>

namespace Workflow {
struct TextInfo {
  std::string name;
  std::string code;
  uint64_t address;
};

std::pair<StateMachine::event, TextInfo> parse_text(std::string &text);

void handle_actions(std::vector<StateMachine::action> &actions, ObjectFile *obj,
                    TextInfo &info);

void run(std::string &text, ObjectFile *obj, StateMachine &sm);
} // namespace Workflow
