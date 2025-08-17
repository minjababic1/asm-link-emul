#pragma once

#include "../inc/types.hpp"
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace SymTabLayout{
  extern const std::size_t NUM_OFF;
  extern const std::size_t NUM_WIDTH;

  extern const std::size_t VAL_OFF;
  extern const std::size_t VAL_WIDTH;

  extern const std::size_t SZ_OFF;
  extern const std::size_t SZ_WIDTH;

  extern const std::size_t TYPE_OFF;
  extern const std::size_t TYPE_WIDTH;

  extern const std::size_t BIND_OFF;
  extern const std::size_t BIND_WIDTH;

  extern const std::size_t SCTN_OFF;
  extern const std::size_t SCTN_WIDTH;

  extern const std::size_t NAME_OFF;
}; // namespace

namespace RelaLayout {
    extern const std::size_t OFFSET_OFF;
    extern const std::size_t OFFSET_WIDTH;

    extern const std::size_t TYPE_OFF;
    extern const std::size_t TYPE_WIDTH;

    extern const std::size_t SYMBOL_OFF;
    extern const std::size_t SYMBOL_WIDTH;

    extern const std::size_t ADDEND_OFF;
    extern const std::size_t ADDEND_WIDTH;
}; // namespace

extern const std::string UNDEFINED_SCTN;
extern const std::string RELA_SCTN_PREFIX; 
extern const std::size_t RELA_SCTN_NAME_OFF;
extern const std::size_t SCTN_NAME_OFF;

extern std::unordered_map<std::string, SymbolBinding> sym_bind_to_str_map;
extern std::unordered_map<std::string, SymbolType> sym_type_to_str_map;
extern std::unordered_map<std::string, RelocationType> rela_type_to_str_map;

std::ostream& operator<<(std::ostream& os, SymbolBinding binding);
std::ostream& operator<<(std::ostream& os, SymbolType type);
std::ostream& operator<<(std::ostream& os, RelocationType reloc);

void updateByte(
  SectionDataTable& a_section_data_table, 
  const std::string& a_sctn_name, 
  uint32_t a_offset, 
  uint8_t a_byte
);

void updateWord(
  SectionDataTable& a_section_data_table, 
  const std::string& a_sctn_name, 
  uint32_t a_offset,
  uint32_t a_word
);

void writeSymTab(std::ostream& a_out, SymbolTable& a_sym_tab);
void writeRela(
  std::ostream& a_out, 
  SectionRelasTable& a_section_relas_table, 
  std::vector<std::string>& a_sections
);
void writeSections(
  std::ostream& a_out, 
  SectionDataTable& a_section_data_table,
  std::vector<std::string>& a_sections,
  SymbolTable& a_sym_tab,
  bool a_hex_mode
);
