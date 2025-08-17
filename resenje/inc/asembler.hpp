#pragma once

#include "common.hpp"
#include "types.hpp"

void insertSymbolIfAbsent(Sym a_sym);
bool symbolDefined(const std::string& a_sym_name);
void openNewSection(std::string a_sctn_name);
void closeCurrentSection();
void writeByte(uint8_t a_byte);
void writeWord(uint32_t a_word);
void handleInstructionSymbol(const std::string& a_sym_name);
void handleDirectiveSymbol(const std::string& a_sym_name);
void handleInstructionLiteral(uint32_t a_literal);
int8_t applyBackpatching();
void defineSymbol(const std::string& a_sym_name, SymbolType a_type);
EquComputation computeEquValue(const EquRecord& a_equ_record);
void writeInstruction(uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b,
  uint8_t a_reg_c,
  uint16_t a_disp);
void writeInstructionFixedFields(uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b,
  uint8_t a_reg_c
);
std::string getSymbolSection(const std::string& a_sym_name);
bool isSymbolDefined(const std::string& a_sym_name);
uint32_t getSymbolValue(const std::string& a_sym_name);
