#pragma once
#include <vector>
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
enum class state {
  OBJECT,
  SECTION,
  FUNCTION,
  BASICBLOCK,
  WAIT,
};
enum class act {
  CRT_SECT,
  CRT_FUNC,
  CRT_BB,
  CRT_INST,
  CRT_PAD,
};
std::vector<act> transition(state &s, event e);

