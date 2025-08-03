#include "../inc/asembler_dir.hpp"
#include <iostream>

void global_(const std::string& a_sym_name){
  insertSymbolIfAbsent(Sym(a_sym_name, SymbolBinding::GLOB));
}

void extern_(const std::string& a_sym_name){
  insertSymbolIfAbsent(Sym(a_sym_name, SymbolBinding::GLOB, SymbolType::NOTYP, UNDEFINED_SCTN));
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

void end_(){
  closeCurrentSection();
}
