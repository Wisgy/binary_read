#include "binary_arch.hh"
#include <cstdint>

uint64_t BasicBlock::hash() {
  if (_hash != UINT64_MAX) {
    return _hash;
  }
  std::hash<std::string> hasher;
  uint64_t seed = 0;
  for (const auto &inst : _instructions) {
    auto str = inst.get_code();
    seed ^= hasher(str) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return _hash = seed;
}

void Padding::print(std::ostream &os) {
  os << "Function: " << _parent->get_name() << "  offset: " << _offset
     << std::endl;
}

void BasicBlock::print(std::ostream &os) {
  os << "Function: " << _parent->get_name() << "  offset: " << _offset
     << "  hash: " << hash() << std::endl;
}

void Function::print(std::ostream &os) {
  for (auto bb : _basic_blocks) {
    bb.print(os);
  }
  for (auto pad : _padding) {
    pad.print(os);
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