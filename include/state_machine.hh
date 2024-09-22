#pragma once
#include <vector>
class StateMachine {
public:
  enum class event {
    NONE,
    NEWLINE,
    NEWSECT,
    NEWFUNC,
    INST,
    BRANCH,
    PAD,
    PATH,
  };

  enum class action {
    CRT_SECT,
    CRT_FUNC,
    CRT_BB,
    CRT_INST,
    CRT_PAD,
  };

  std::vector<action> get_next_action(event e);

  StateMachine() : s(state::OBJECT) {}

private:
  enum class state {
    OBJECT,
    SECTION,
    FUNCTION,
    BASICBLOCK,
    WAIT,
  };

  std::vector<action> transition(event e);

  state s;
};
