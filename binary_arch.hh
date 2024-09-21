
#pragma once

#include <cstdint>
#include <list>
#include <string>

class Instruction;
class BasicBlock;
class Function;
class Section;
class ObjectFile;

class Instruction {
private:
  uint64_t _address;
  std::string _code;
  BasicBlock *_basic_block;
  Function *_function;

public:
  Instruction(BasicBlock *parent, uint64_t address, std::string code)
      : _address(address), _code(code), _basic_block(parent) {}
  Instruction(Function *parent, uint64_t address, std::string code)
      : _address(address), _code(code), _function(parent) {}
};

class BasicBlock {
private:
  uint64_t _offset;
  uint64_t _hash;
  std::list<Instruction> _instructions;
  Function *_parent;

public:
  BasicBlock(Function *parent, uint64_t offset)
      : _offset(offset), _parent(parent) {}

  template <typename... Args> Instruction *create_instruction(Args... args) {
    _instructions.emplace_back(this, args...);
    return &_instructions.back();
  }

  void hash(); // TODO: implement
};

class Function {
private:
  std::string _name;
  uint64_t _start_address;
  std::list<BasicBlock> _basic_blocks;
  std::list<Instruction> _padding;
  Section *_parent;

public:
  Function(Section *parent, std::string name, uint64_t start_address)
      : _name(name), _start_address(start_address), _parent(parent) {}

  template <typename... Args> BasicBlock *create_basic_block(Args... args) {
    _basic_blocks.emplace_back(this, args...);
    return &_basic_blocks.back();
  }
  template <typename... Args> Instruction *create_padding(Args... args) {
    _padding.emplace_back(this, args...);
    return &_padding.back();
  }
  uint64_t get_start_address() { return _start_address; }
};
class Section {
private:
  std::string _name;
  std::list<Function> _functions;
  ObjectFile *_parent;

public:
  Section(ObjectFile *parent, std::string name)
      : _name(name), _parent(parent) {}

  template <typename... Args> Function *create_function(Args... args) {
    _functions.emplace_back(this, args...);
    return &_functions.back();
  }
};

class ObjectFile {
public:
  template <typename... Args> Section *create_section(Args... args) {
    _sections.emplace_back(this, args...);
    return &_sections.back();
  }

private:
  std::list<Section> _sections;
};