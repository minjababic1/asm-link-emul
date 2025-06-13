#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>

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
