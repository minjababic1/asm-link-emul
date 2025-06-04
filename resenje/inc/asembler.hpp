#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unordered_map>
#include <vector>

// enum RelocationType{
//   R_X86_64_64,
//   R_X86_64_PC64,
//   R_X86_64_PC32,
//   R_X86_64_32,
//   R_X86_64_32S,
//   R_X86_64_16,
//   R_X86_64_PC16,
//   R_X86_64_PC8,
//   R_X86_64_8
// };

// enum SymbolBinding{
//   LOCAL,
//   GLOBAL,
//   UNKNOWN_BINDING
// };

// enum SymbolType{
//   NOTYPE,
//   SECTION,
//   OBJECT,
//   FUNCTION,
//   UNKNOWN_TYPE
// };

// struct Section{
//   char* m_data;
//   // add general info
//   Section(char* a_data): m_data(a_data) {}
// };

// struct Symbol{
//   uint32_t m_num;
//   uint32_t m_value;
//   SymbolType m_type;
//   SymbolBinding m_bind;
//   uint32_t m_index;
//   Symbol(uint32_t a_num, uint32_t a_value, SymbolType a_type, SymbolBinding a_bind, uint32_t a_index): 
//     m_num(a_num), m_value(a_value), m_type(a_type), m_bind(a_bind), m_index(a_index) {}
// };

// struct Relocation{
//   uint32_t m_offset;
//   RelocationType m_type;
//   uint32_t m_addend;
//   Relocation(uint32_t a_offset, RelocationType a_type, uint32_t a_addend):
//     m_offset(a_offset), m_type(a_type), m_addend(a_addend) {}
// };

// std::unordered_map<std::string, Relocation> relocation_table;
// std::unordered_map<std::string, Symbol> symbol_table;
// std::vector<Section> sections;
// uint32_t lc;
void printSymbolLists();
void insertIntoCurrentDirectiveList(const char* symbol);
// void insertIntoSymbolTable(std::string a_name, Symbol a_symbol);
// void insertIntoRelocationTable(std::string a_name, Relocation a_relocation);

extern std::vector<std::string> global_symbols;
extern std::vector<std::string> extern_symbols;

extern std::string current_directive;
