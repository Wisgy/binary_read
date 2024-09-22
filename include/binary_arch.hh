
#pragma once

#include <cstdint>
#include <iostream>
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
  BasicBlock *_parent;

public:
  Instruction(BasicBlock *parent, uint64_t address, std::string code)
      : _address(address), _code(code), _parent(parent) {}

  std::string get_code() const { return _code; }
};

class Padding {
private:
  uint64_t _offset;
  Function *_parent;

public:
  Padding(Function *parent, uint64_t offset)
      : _offset(offset), _parent(parent) {}

  void print(std::ostream &os);
};

class BasicBlock {
private:
  uint64_t _offset;
  uint64_t _hash = UINT64_MAX;
  std::list<Instruction> _instructions;
  Function *_parent;

public:
  BasicBlock(Function *parent, uint64_t offset)
      : _offset(offset), _parent(parent) {}

  template <typename... Args> Instruction *create_instruction(Args... args) {
    _instructions.emplace_back(this, args...);
    return &_instructions.back();
  }

  uint64_t hash();

  void print(std::ostream &os);
};

class Function {
private:
  std::string _name;
  uint64_t _start_address;
  std::list<BasicBlock> _basic_blocks;
  std::list<Padding> _padding;
  Section *_parent;

public:
  Function(Section *parent, std::string name, uint64_t start_address)
      : _name(name), _start_address(start_address), _parent(parent) {}

  template <typename... Args> BasicBlock *create_basic_block(Args... args) {
    _basic_blocks.emplace_back(this, args...);
    return &_basic_blocks.back();
  }
  template <typename... Args> Padding *create_padding(Args... args) {
    _padding.emplace_back(this, args...);
    return &_padding.back();
  }

  uint64_t get_start_address() { return _start_address; }

  std::string get_name() const { return _name; }

  void print(std::ostream &os);
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

  void print(std::ostream &os);
};

class ObjectFile {
public:
  template <typename... Args> Section *create_section(Args... args) {
    _sections.emplace_back(this, args...);
    return &_sections.back();
  }
  ObjectFile() = default;

  void print(std::ostream &os);

private:
  std::list<Section> _sections;
};