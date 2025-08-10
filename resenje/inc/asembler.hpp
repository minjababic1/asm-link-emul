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
 * @brief Returns wheter symbol is defined in symbol table or not
 * 
 * @param a_sym_name Name of the given symbol
 */
bool symbolDefined(const std::string& a_sym_name);

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
 * @brief Computes the value of the EQU directive if possible
 * 
 * @param a_equ_record EQU record containing the symbol and operands
 * @return EquComputation Result of the computation, indicating if it was computable and the
 * computed value
 */
EquComputation computeEquValue(const EquRecord& a_equ_record);

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

/**
 * @brief Gets the section name of the given symbol
 * 
 * @param a_sym_name Name of the given symbol
 * @return std::string Name of the section where the symbol is defined
 */
std::string getSymbolSection(const std::string& a_sym_name);

/**
 * @brief Checks if the symbol is defined in the symbol table
 * 
 * @param a_sym_name Name of the given symbol
 * @return true If the symbol is defined
 * @return false If the symbol is not defined
 */
bool isSymbolDefined(const std::string& a_sym_name);

/**
 * @brief Gets the value of the symbol from the symbol table
 * 
 * @param a_sym_name Name of the given symbol
 * @return uint32_t Value of the symbol
 */
uint32_t getSymbolValue(const std::string& a_sym_name);

