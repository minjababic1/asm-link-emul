#pragma once

#include <climits>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Symbol visibility (binding) type
 */
enum SymbolBinding {
  /// Local symbol
  LOC,  

  /// Global symbol
  GLOB  
};

/**
 * @brief Symbol type classification
 */
enum SymbolType {
  /// No specific type
  NOTYP, 

  /// Type given to the sections
  SCTN,   

  /// Type given to the symbols
  OBJ     
};

/**
 * @brief Type of relocation to apply during linking
 */
enum RelocationType {
  /// Absolute 32-bit address of symbol
  R_X86_64_32
};

/**
 * @brief Represents a forward reference to a symbol not yet defined
 */
struct ForwardReferenceEntry {
  /// Section where the reference occurs
  std::string m_sctn_name;       

  /// Offset within the section needing relocation
  uint32_t m_offset;              

  /// Additional constant to be added during relocation
  int32_t m_addend;

  ForwardReferenceEntry(const std::string& a_sctn_name, 
    uint32_t a_offset,
    int32_t a_addend) 
    : m_sctn_name(a_sctn_name),
      m_offset(a_offset),
      m_addend(a_addend) {}
};

/**
 * @brief Represents a symbol table entry
 */
struct Sym {
  /// Symbol index
  uint32_t m_index;

  /// Symbol name
  std::string m_name;                   

  /// Symbol binding
  SymbolBinding m_bind;                           

  /// Symbol type
  SymbolType m_type;                 

  /// Name of the section the symbol belongs to             
  std::string m_sctn_name;                        

  /// Value of the symbol
  uint32_t m_value;                            

  /// Whether the symbol is defined
  bool m_defined;                                 

  /// List of unresolved references to this symbol
  std::vector<ForwardReferenceEntry> m_forward_ref_table; 

  Sym(const std::string& name = "",
        SymbolBinding bind = SymbolBinding::LOC,
        SymbolType type = SymbolType::NOTYP,
        const std::string& sctn_name = "",
        uint32_t value = 0,
        bool defined = false)
        : m_index(UINT_MAX),
          m_name(name),
          m_bind(bind),
          m_type(type),
          m_sctn_name(sctn_name),
          m_value(value),
          m_defined(defined) {}
};

/**
 * @brief Represents a relocation entry to be resolved during linking
 */
struct Rela {
  /// Offset within the section where relocation is to be applied
  uint32_t m_offset;

  /// Name of the symbol being referenced
  std::string m_sym_name;

  /// Type of relocation to perform
  RelocationType m_rela_type;

  /// Additional constant to be added during relocation
  int32_t m_addend;

  Rela(uint32_t a_offset, 
        std::string a_sym_name, 
        RelocationType a_rela_type, 
        int32_t a_addend) 
        : m_offset(a_offset),
          m_sym_name(a_sym_name),
          m_rela_type(a_rela_type),
          m_addend(a_addend) {}           
};

struct SectionPlace{
  std::string m_sctn_name;
  uint32_t m_addr;
  SectionPlace(std::string a_sctn_name, uint32_t a_addr)
    : m_sctn_name(a_sctn_name), m_addr(a_addr) {}
};

struct EquOperand{
  bool m_is_literal;
  std::string m_representation;
  EquOperand(bool a_is_literal, std::string a_representation)
    : m_is_literal(a_is_literal), m_representation(a_representation) {}
};

struct EquRecord{
  std::string m_equ_symbol;
  std::vector<EquOperand> m_operands;
  std::vector<char> m_operations;
  EquRecord(
    std::string a_equ_symbol,
    std::vector<EquOperand> a_operands,
    std::vector<char> a_operations) 
    : m_equ_symbol(a_equ_symbol), m_operands(a_operands), m_operations(a_operations) {}
};

struct EquComputation{
  bool m_is_computable;
  int32_t m_value;
  EquComputation(bool a_is_computable, int32_t a_value)
    : m_is_computable(a_is_computable), m_value(a_value) {}
};

struct EquUsage{
  std::string m_equ_symbol;
  SectionPlace m_section_place;
  EquUsage(std::string a_equ_symbol, SectionPlace a_section_place)
    : m_equ_symbol(a_equ_symbol), m_section_place(a_section_place) {}
};

using SymbolTable = std::unordered_map<std::string, Sym>;
using SectionRelasTable = std::unordered_map<std::string, std::vector<Rela>>;
using SectionDataTable = std::unordered_map<std::string, std::vector<uint8_t>>;
using LiteralUsagesTable = std::unordered_map<uint32_t, std::vector<uint32_t>>;
using SectionLiteralsTable = std::unordered_map<std::string, std::vector<uint32_t>>;
using SymbolUsagesTable = std::unordered_map<std::string, std::vector<uint32_t>>;
using SectionSymbolsTable = std::unordered_map<std::string, std::vector<std::string>>;
using SectionPlaceTable = std::vector<SectionPlace>;
using SymbolList = std::vector<Sym>; 
using NonComputableSymbolTable = std::vector<EquRecord>;
using EquUsages = std::vector<EquUsage>;

