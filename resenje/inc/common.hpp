#pragma once

#include "../inc/types.hpp"
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

/**
 * @brief Type of section in an ELF file.
 */
enum SectionType
{
  /// Section with program data.
  PRG_DATA,
  /// Section with symbol table.
  SYMTAB,
  /// Section with relocation records.
  RELA
};

/**
 * @brief Represent the header of an ELF file
 */
struct ElfHdr
{
  /// Offset to the section header table from the start of the ELF file.
  uint64_t e_shoff;

  /// Size of the ELF header in bytes.
  uint16_t e_ehsize;

  /// Size of a single entry in the section header table.
  uint16_t e_shentsize;

  /// Number of entries in the section header table.
  uint16_t e_shnum;
};

/**
 * @brief Represents a section header in an ELF file.
 */
struct SectionHdr
{
  /// Name of the section.
  std::string sh_name;

  /// Type of the section (e.g., PROGBITS, SYMTAB).
  SectionType sh_type;

  /// Offset from the start of the ELF file to the start of the section data.
  uint32_t sh_offset;

  /// Size of the section in bytes.
  uint64_t sh_size;

  /// Index of a related section in the section header table.
  uint32_t sh_link;

  /// Additional information, usage depends on the section type.
  uint32_t sh_info;

  /// Size of each entry in sections that contain tables (e.g., SYMTAB, RELA).
  uint32_t sh_entsize;
};

extern ElfHdr elf_hdr;
extern std::unordered_map<std::string, SectionHdr> section_headers;

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
};

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

/**
 * @brief Overwrites a byte at the specified offset in the given section
 * 
 * @param a_section_data_table Maps section to it's content (data)
 * @param a_sctn_name Name of the section to update
 * @param a_offset Offset within the section where the byte will be written
 * @param a_byte Byte value to write at the specified location
 */
void updateByte(
  SectionDataTable& a_section_data_table, 
  const std::string& a_sctn_name, 
  uint32_t a_offset, 
  uint8_t a_byte
);

/**
 * @brief Overwrites a word at the specified offset in the given section
 * 
 * @param a_section_data_table Maps section to it's content (data)
 * @param a_sctn_name Name of the section to update
 * @param a_offset Offset within the section where the byte will be written
 * @param a_word 4-Byte value to write at the specified location
 */
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
