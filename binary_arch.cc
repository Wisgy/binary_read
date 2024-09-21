#include "binary_arch.hh"
#include <cstdint>

uint64_t BasicBlock::hash() {
  // TODO: implement
  return 0;
}

void BasicBlock::print(std::ostream &os) {
  os << "Function: " << _parent->get_name() << "  offset: " << _offset
     << "  hash: " << hash() << std::endl;
}

void Function::print(std::ostream &os) {
  for (auto bb : _basic_blocks) {
    bb.print(os);
  }
}

void Section::print(std::ostream &os) {
  os << "Section: " << _name << std::endl;
  for (auto func : _functions) {
    func.print(os);
  }
}

void ObjectFile::print(std::ostream &os) {
  for (auto sec : _sections) {
    sec.print(os);
  }
}