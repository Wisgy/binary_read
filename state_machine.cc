#include "state_machine.hh"
#include <stdexcept>
#include <utility>

std::vector<StateMachine::action> StateMachine::transition(event e) {
  switch (s) {
  case state::OBJECT:
    switch (e) {
    case event::NEWSECT:
      s = state::SECTION;
      return {action::CRT_SECT};
    case event::NEWLINE:
    case event::PATH:
      return {};
    default:
      goto error;
    }
  case state::SECTION:
    switch (e) {
    case event::NEWFUNC:
      s = state::FUNCTION;
      return {action::CRT_FUNC};
    case event::NEWLINE:
      return {};
    default:
      goto error;
    }
  case state::FUNCTION:
    switch (e) {
    case event::NEWLINE:
      s = state::WAIT;
      return {};
    case event::INST:
      s = state::BASICBLOCK;
      return {action::CRT_BB, action::CRT_INST};
    case event::BRANCH:
      return {action::CRT_BB, action::CRT_INST};
    case event::PAD:
      return {action::CRT_PAD};
    default:
      goto error;
    }
  case state::BASICBLOCK:
    switch (e) {
    case event::INST:
      return {action::CRT_INST};
    case event::BRANCH:
      s = state::FUNCTION;
      return {action::CRT_INST};
    case event::PAD:
      return {action::CRT_PAD};
    case event::NEWLINE: // FIXME: need to modify the state_machine
      s = state::WAIT;
      return {};
    default:
      goto error;
    }
  case state::WAIT:
    switch (e) {
    case event::NEWFUNC:
      s = state::FUNCTION;
      return {action::CRT_FUNC};
    case event::NEWSECT:
      s = state::SECTION;
      return {action::CRT_SECT};
    case event::NEWLINE:
      return {};
    default:
      goto error;
    }
  }
error:
  throw std::runtime_error("Wrong event");
}

std::vector<StateMachine::action> StateMachine::get_next_action(event e) {
  return std::move(transition(e));
}
