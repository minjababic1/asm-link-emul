#include "../inc/asembler_dir.hpp"
#include <iostream>

extern SymbolTable sym_tab;
extern NonComputableSymbolTable non_computable_symbols;

void global_(const std::string& a_sym_name){
  insertSymbolIfAbsent(Sym(a_sym_name, SymbolBinding::GLOB));
  Sym sym = sym_tab[a_sym_name];
  sym.m_bind = SymbolBinding::GLOB;
  sym_tab[a_sym_name] = sym;
}

void extern_(const std::string& a_sym_name){
  insertSymbolIfAbsent(Sym(a_sym_name, SymbolBinding::GLOB, SymbolType::NOTYP, UNDEFINED_SCTN));
  Sym sym = sym_tab[a_sym_name];
  sym.m_bind = SymbolBinding::GLOB;
  sym.m_sctn_name = UNDEFINED_SCTN;
  sym.m_type = SymbolType::NOTYP;
  sym_tab[a_sym_name] = sym;
}

void word_(const std::string& a_sym_name){
  handleDirectiveSymbol(a_sym_name);
}

void word_(uint32_t a_literal){
  writeWord(static_cast<uint32_t>(a_literal));
}

void section_(const std::string& a_sym_name){
  closeCurrentSection();
  openNewSection(a_sym_name);
  defineSymbol(a_sym_name, SymbolType::SCTN);
}

void skip_(uint32_t a_literal){
  for(uint8_t i = 0; i < a_literal; i++){
        writeByte(0x00);
  }
}

void ascii_(const std::string& a_word) {
  for (char c : a_word) {
    writeByte(c);
  }
}

void equ_(EquRecord a_equ_record) {
  insertSymbolIfAbsent(Sym(a_equ_record.m_equ_symbol));
  Sym sym = sym_tab[a_equ_record.m_equ_symbol];
  EquComputation equ_computation = computeEquValue(a_equ_record);
  if (!equ_computation.m_is_computable) {
    non_computable_symbols.push_back(a_equ_record);
    sym.m_sctn_name = "#EQU";
    sym.m_defined = false;
    sym_tab[a_equ_record.m_equ_symbol] = sym;
  } else {
    sym.m_value = equ_computation.m_value;
    sym.m_defined = true;
    sym.m_sctn_name = "#EQU";
    sym_tab[a_equ_record.m_equ_symbol] = sym;
  }
}

void end_(){
  closeCurrentSection();
}
