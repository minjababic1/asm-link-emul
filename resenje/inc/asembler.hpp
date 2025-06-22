#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
  NOTYPE, 

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
  R_X86_64_32,   

  /// PC-relative 32-bit offset from current location to symbol
  R_X86_64_PC32  
};

/**
 * @brief Represents a forward reference to a symbol not yet defined
 */
struct ForwardReferenceEntry {
  /// Section where the reference occurs
  std::string m_sctn_name;       

  /// Offset within the section needing relocation
  uint32_t m_offset;             

  /// Type of relocation to apply
  RelocationType m_reloc_type;   

  /// Additional constant to be added during relocation
  int32_t m_addend;

  ForwardReferenceEntry(const std::string& a_sctn_name, 
    uint32_t a_offset,
    RelocationType a_reloc_type,
    int32_t a_addend) 
    : m_sctn_name(a_sctn_name),
      m_offset(a_offset),
      m_reloc_type(a_reloc_type),
      m_addend(a_addend) {}
};

/**
 * @brief Represents a symbol table entry
 */
struct Sym {
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
        SymbolType type = SymbolType::NOTYPE,
        const std::string& sctn_name = "",
        uint32_t value = 0,
        bool defined = false)
        : m_name(name),
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

/**
 * @brief Represents the usage of the literal 
 */
struct LiteralUsage{
  /// Represents the place where literal is used
  uint32_t m_offset;

  /// Represents the literal value
  uint32_t m_literal;
};

/**
 * @brief Adds symbol to the symbol table
 * 
 * @param a_sym Symbol that needs to be added
 */
void addSymbol(Sym a_sym);

/**
 * @brief Declares new section as the current section
 * 
 * @param a_sctn_name Name of the new section
 */
void openNewSection(std::string a_sctn_name);

/**
 * @brief Generates literal pool for the current section
 */
void closeCurrentSection();

/**
 * @brief Writes one byte in the current section
 * 
 * @param a_byte Byte that needs to be written
 */
void writeByte(uint8_t a_byte);

/**
 * @brief Writes one machine word(4 bytes) in the current section
 * 
 * @param a_word Word that needs to be written
 */
void writeWord(uint32_t a_word);

/**
 * @brief Handles usage of the symbol in the instruction or directive based on the given parameters
 * 
 * @param a_sym_name Symbol which usage needs to be handled
 * @param a_reloc_type Type of relocation, telling us how symbol is being used
 * @param a_reg_c Value that needs to be written on the place of regC in the instruction
 * @param a_addend Addend that needs to be written in relocation
 */
void handleSymbolUsage(const std::string& a_sym_name, RelocationType a_reloc_type, uint8_t a_reg_c, int32_t a_addend);

/**
 * @brief Handles forward reference for the every symbol in the symbol table
 */
int8_t applyBackpatching();

/**
 * @brief Updates information about symbol in symbol table
 * 
 * @param a_sym_name Name of the given symbol
 */
void defineSymbol(const std::string& a_sym_name);

/**
 * @brief Handles usage of the literal in the instruction or directive
 * 
 * @param a_literal Literal which usage needs to be handled
 * @param a_reg_c Value that needs to be written on the place of regC in the instructions
 */
void handleLiteralUsage(uint32_t a_literal, uint8_t a_reg_c);

/**
 * @brief Writes instruction with the given parameters
 * 
 * @param a_oc Operation code
 * @param a_mod Modificator of the instruction
 * @param a_reg_a Index of the first gpr used in the instruction
 * @param a_reg_b Index of the second gpr used in the instruction
 * @param a_reg_c Index of the third gpr used in the instruction
 * @param a_disp Displacement
 */
void writeInstruction(uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b,
  uint8_t a_reg_c,
  uint16_t a_disp);

/**
 * @brief Writes first half of the instruction with the given parameters
 * 
 * @param a_oc Operation code
 * @param a_mod Modificator of the instruction
 * @param a_reg_a Index of the first gpr used in the instruction
 * @param a_reg_b Index of the second gpr used in the instruction
 */
void writeInstructionFixedFields(uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b,
  uint8_t a_reg_c
);

extern uint32_t location_counter;
extern std::unordered_map<std::string, Sym> sym_tab;
extern const std::string UNDEFINED_SCTN;

/**
 * @brief temporary functions
 */
void printSymbolTable();
void printRela();
void printSections();
void printAll();
