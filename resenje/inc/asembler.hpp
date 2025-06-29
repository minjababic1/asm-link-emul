#pragma once

#include "common.hpp"
#include "types.hpp"

/**
 * @brief Inserts symbol only if it does not already exist in the symbol table.
 * 
 * @param a_sym Symbol to insert.
 */
void insertSymbolIfAbsent(Sym a_sym);

/**
 * @brief Declares new section as the current section
 * 
 * @param a_sctn_name Name of the new section
 */
void openNewSection(std::string a_sctn_name);

/**
 * @brief Generates literal and symbol pool for the current section
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
 * @brief Handles usage of the symbol in the instruction
 * 
 * @param a_sym_name Name of the given symbol
 */
void handleInstructionSymbol(const std::string& a_sym_name);

/**
 * @brief Handles usage of the symbol in the directive
 * 
 * @param a_sym_name Name of the given symbol
 */
void handleDirectiveSymbol(const std::string& a_sym_name);

/**
 * @brief Handles usage of the literal in the instruction
 * 
 * @param a_literal Given literal
 */
void handleInstructionLiteral(uint32_t a_literal);

/**
 * @brief Handles forward reference for the every symbol in the symbol table
 */
int8_t applyBackpatching();

/**
 * @brief Updates information about symbol in symbol table
 * 
 * @param a_sym_name Name of the given symbol
 * @param a_type Symbol type
 */
void defineSymbol(const std::string& a_sym_name, SymbolType a_type);

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
 * @brief Writes all parts of the instruction except the 12-bit disp field
 * 
 * @param a_oc Operation code
 * @param a_mod Modificator of the instruction
 * @param a_reg_a Index of the first gpr used in the instruction
 * @param a_reg_b Index of the second gpr used in the instruction
 * @param a_reg_c Index of the third gpr used in the instruction
 */
void writeInstructionFixedFields(uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b,
  uint8_t a_reg_c
);
