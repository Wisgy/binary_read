#include "state_machine.hh"
#include <stdexcept>

std::vector<act> transition(state &s, event e) {
  switch (s) {
  case state::OBJECT:
    switch (e) {
    case event::NEWSECT:
      s = state::SECTION;
      return {act::CRT_SECT};
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
      return {act::CRT_FUNC};
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
      return {act::CRT_BB, act::CRT_INST};
    case event::BRANCH:
      return {act::CRT_BB, act::CRT_INST};
    case event::PAD:
      return {act::CRT_PAD};
    default:
      goto error;
    }
  case state::BASICBLOCK:
    switch (e) {
    case event::INST:
      return {act::CRT_INST};
    case event::BRANCH:
      s = state::FUNCTION;
      return {act::CRT_INST};
    case event::PAD:
      return {act::CRT_PAD};
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
      return {act::CRT_FUNC};
    case event::NEWSECT:
      s = state::SECTION;
      return {act::CRT_SECT};
    case event::NEWLINE:
      return {};
    default:
      goto error;
    }
  }
error:
  throw std::runtime_error("Wrong event");
}
