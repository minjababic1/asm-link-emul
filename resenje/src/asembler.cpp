#include "../inc/asembler.hpp"
#include <iostream>

std::unordered_map<std::string, Sym> sym_tab;
std::unordered_map<std::string, std::vector<Rela>> rela_table;
std::unordered_map<std::string, std::vector<uint8_t>> section_data_table;
std::unordered_map<uint32_t, std::vector<uint32_t>> literal_usage_table;
std::unordered_map<std::string, std::vector<uint32_t>> literal_pool;
std::unordered_map<std::string, std::vector<uint32_t>> symbol_usage_table;
std::unordered_map<std::string, std::vector<std::string>> symbol_pool;

std::string current_section = "";
uint32_t location_counter = 0;
uint32_t total_offset = 0;
const std::string UNDEFINED_SCTN = "UND";
const uint8_t INSTR_ADDEND = 2;
const uint8_t DIR_ADDEND = 0;

void addSymbol(Sym a_sym){
  if( sym_tab.find(a_sym.m_name) == sym_tab.end()) {
    sym_tab[a_sym.m_name] = a_sym;
  }
}

void openNewSection(std::string a_sctn_name){
  current_section = a_sctn_name;
  location_counter = 0;
  literal_usage_table.clear();
}

void closeCurrentSection(){
  // jump over literal and symbol pool
  if(literal_usage_table.size() > 0 || symbol_usage_table.size() > 0){
    writeInstruction(0x03, 0x00, 0x0F, 0x00, 0x00, (literal_usage_table.size()+symbol_usage_table.size())*4);
  }

  // make usage of literal point to literal in the pool
  for(const auto& kv : literal_usage_table){
    writeWord(kv.first);
    for(const auto& usage_addr : kv.second){
      uint16_t disp = location_counter - usage_addr - 2;
      section_data_table[current_section][usage_addr] |= static_cast<uint8_t>((disp >> 8) & 0x0F);
      section_data_table[current_section][usage_addr+1] |= static_cast<uint8_t>(disp & 0x00FF);
    }
    literal_pool[current_section].push_back(kv.first);
  }

  for(const auto& kv : symbol_usage_table){
    std::string sym_name = kv.first;
    
  }
}

void writeByte(uint8_t a_byte){
  section_data_table[current_section].push_back(a_byte);
  adjustLocation(1);
}

void writeWord(uint32_t a_word){
  writeByte(static_cast<uint8_t>(a_word & 0xFF));
  writeByte(static_cast<uint8_t>((a_word >> 8) & 0xFF));
  writeByte(static_cast<uint8_t>((a_word >> 16) & 0xFF));
  writeByte(static_cast<uint8_t>((a_word >> 24) & 0xFF));
}

/**
 * @brief Updates the location counter in the 
 * current section for the given number of bytes
 * 
 * @param a_bytes Given number of bytes
 */
void adjustLocation(uint32_t a_bytes){
  location_counter+= a_bytes;
  total_offset+= a_bytes;
}

void handleSymbolUsage(const std::string& a_sym_name, RelocationType a_rela_type, uint8_t a_reg_c, int32_t a_addend){
  if(sym_tab.find(a_sym_name) != sym_tab.end() && sym_tab[a_sym_name].m_defined){
    Sym sym = sym_tab[a_sym_name];
    if(sym.m_sctn_name == current_section && a_rela_type == RelocationType::R_X86_64_PC32) {
      uint16_t disp = sym.m_value - location_counter - a_addend;
      writeByte((a_reg_c << 4) | (static_cast<uint8_t>(disp >> 8) & 0x0F));
      writeByte(static_cast<uint8_t>(disp & 0x00FF));
      return;
    } else{
      Rela rela = Rela(location_counter, a_sym_name, a_rela_type, a_addend);
      if(sym.m_bind == SymbolBinding::LOC){
        rela.m_sym_name = sym.m_sctn_name;
        rela.m_addend+= sym.m_value;
      }
      rela_table[current_section].push_back(rela);
      if(a_rela_type == RelocationType::R_X86_64_PC32){
        uint16_t disp = sym.m_value - location_counter - a_addend;
        writeByte((a_reg_c << 4) | (static_cast<uint8_t>(disp >> 8) & 0x0F));
        writeByte(static_cast<uint8_t>(disp & 0x00FF));
        return;
      } else{
        writeWord(sym.m_value);
        return;
      }
    }
  } else{
    Sym sym;
    if(sym_tab.find(a_sym_name) == sym_tab.end()){
      sym = Sym(a_sym_name);
      addSymbol(sym);
    } else {
      sym = sym_tab[a_sym_name];
    }
    sym.m_forward_ref_table.push_back(
      ForwardReferenceEntry(
        current_section,
        location_counter,
        a_rela_type,
        a_addend
      )
    );
    sym_tab[a_sym_name] = sym;
    if(a_rela_type == RelocationType::R_X86_64_PC32){
        uint16_t disp = sym.m_value - location_counter - a_addend;
        writeByte((a_reg_c << 4) & 0xF0);
        writeByte(0x00);
        return;
      } else{
        writeWord(0x00000000);
        return;
    }
  }
}

/**
 * @brief Updates content of the given section based on the other parameters
 * 
 * @param a_sctn_name Name of the updated section
 * @param a_offset Place where section is updating
 * @param a_word New content that is gonna be written
 * @param a_reloc_type Type of relocation, determinates what is the width of the given content
 */
void updateSection(const std::string& a_sctn_name, uint32_t a_offset, uint32_t a_word, RelocationType a_reloc_type){
  if(a_reloc_type == RelocationType::R_X86_64_32){
    section_data_table[a_sctn_name][a_offset] = static_cast<uint8_t>(a_word & 0xFF);
    section_data_table[a_sctn_name][a_offset+1] = static_cast<uint8_t>((a_word >> 8) & 0xFF);
    section_data_table[a_sctn_name][a_offset+2] = static_cast<uint8_t>((a_word >> 16) & 0xFF);
    section_data_table[a_sctn_name][a_offset+3] = static_cast<uint8_t>((a_word >> 24) & 0xFF);
  } else if(a_reloc_type == RelocationType::R_X86_64_PC32){
    uint16_t disp = static_cast<uint16_t>(a_word);
    uint8_t reg_c = section_data_table[a_sctn_name][a_offset] >> 4;
    section_data_table[a_sctn_name][a_offset] = (reg_c << 4) | (static_cast<uint8_t>(disp >> 8) & 0x0F);
    section_data_table[a_sctn_name][a_offset+1] = static_cast<uint8_t>(disp & 0x00FF);
  } else{
    std::cout<< "ERROR - No appropriate relocation type"<<std::endl;
  }
  
}

int8_t applyBackpatching(){
  for(const auto& kv : sym_tab){
    Sym sym = kv.second;
    if(!sym.m_defined && sym.m_sctn_name != UNDEFINED_SCTN){
      printf("GRESKA\n");
      return -1;
    }
    for(const auto& forward_ref : sym.m_forward_ref_table){
      if(!(forward_ref.m_sctn_name == sym.m_sctn_name && 
        forward_ref.m_reloc_type == RelocationType::R_X86_64_PC32)){
          Rela rela = Rela(forward_ref.m_offset, sym.m_name, forward_ref.m_reloc_type, forward_ref.m_addend);
          if(sym.m_bind == SymbolBinding::LOC){
            rela.m_sym_name = sym.m_sctn_name;
            rela.m_addend+= sym.m_value;
          }
          rela_table[forward_ref.m_sctn_name].push_back(rela);
      }
      uint32_t value = forward_ref.m_reloc_type == RelocationType::R_X86_64_32 ? 
        sym.m_value : sym.m_value - forward_ref.m_offset - forward_ref.m_addend;
      updateSection(forward_ref.m_sctn_name, forward_ref.m_offset, value, forward_ref.m_reloc_type);
    }
  }
  return 0;
}

void handleLiteralUsage(uint32_t a_literal, uint8_t a_reg_c){
  a_reg_c &= 0x0F;
  literal_usage_table[a_literal].push_back(location_counter);
  writeByte((a_reg_c << 4));
  writeByte(0x00);
}

void defineSymbol(const std::string& a_sym_name){
  if(sym_tab.find(a_sym_name) == sym_tab.end()){
    addSymbol(Sym(a_sym_name));
  }
  Sym sym = sym_tab[a_sym_name];
  sym.m_sctn_name = current_section;
  sym.m_value = location_counter;
  sym.m_defined = true;
  sym_tab[a_sym_name] = sym;
}

void writeInstruction(uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b,
  uint8_t a_reg_c,
  uint16_t a_disp) {
    writeInstructionFixedFields(a_oc, a_mod, a_reg_a, a_reg_b, a_reg_c);
    patchDispField(a_disp);
  }

void writeInstructionFixedFields(uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b,
  uint8_t a_reg_c
){
    writeByte((a_oc << 4) | a_mod);
    writeByte((a_reg_a << 4) | a_reg_b);
    writeByte((a_reg_c << 4) & 0xF0);
}

void updateByte(const std::string& a_sctn_name, uint32_t a_offset, uint8_t a_byte){
  section_data_table[a_sctn_name][a_offset] = a_byte;
}

/**
 * @brief Patches the third byte and writes the fourth byte to complete 12-bit displacement.
 * 
 * @param a_disp Displacement value
 **/ 
void patchDispField(uint16_t a_disp) {
    uint8_t regC_bits = section_data_table[current_section][location_counter - 1] & 0xF0;
    uint8_t disp_high = static_cast<uint8_t>((a_disp >> 8) & 0x0F);
    uint8_t third_byte = regC_bits | disp_high;

    updateByte(current_section, location_counter - 1, third_byte);
    writeByte(static_cast<uint8_t>(a_disp & 0x00FF));
}

void addRela(Sym a_sym, Rela& a_rela){
  if(a_sym.m_bind == SymbolBinding::LOC){
    a_rela.m_sym_name = a_sym.m_sctn_name;
    a_rela.m_addend+= a_sym.m_value;
  }
  rela_table[current_section].push_back(a_rela);
}

void addSymUsage(const std::string a_sym_name){
  symbol_usage_table[a_sym_name].push_back(location_counter-1);
}

void addLiteralUsage(uint32_t a_literal){
  literal_usage_table[a_literal].push_back(location_counter-1);
}

void addForwardReference(const std::string a_sym_name,
  uint32_t a_offset,
  RelocationType a_reloc_type,
  int32_t a_addend
){
  Sym sym;
  if(sym_tab.find(a_sym_name) == sym_tab.end()){
    sym = Sym(a_sym_name);
    addSymbol(sym);
  } else {
    sym = sym_tab[a_sym_name];
  }
  sym.m_forward_ref_table.push_back(
    ForwardReferenceEntry(
      current_section,
      a_offset,
      a_reloc_type,
      a_addend
    )
  );
  sym_tab[a_sym_name] = sym;
}

void writeEmptyDisp(){
  writeByte(0x00);
}

/// Expected written first 3 bytes of the instruction
void handleInstructionSymbol(const std::string& a_sym_name){
  if(sym_tab.find(a_sym_name) != sym_tab.end() && sym_tab[a_sym_name].m_defined){
    Sym sym = sym_tab[a_sym_name];
    if(sym.m_sctn_name == current_section) {
      uint32_t total_sym_val = sym_tab[sym.m_sctn_name].m_value + sym.m_value;
      uint16_t disp = total_sym_val - total_offset - INSTR_ADDEND;
      patchDispField(disp);
    } else {
      Rela rela = 
        Rela(location_counter-1, a_sym_name, RelocationType::R_X86_64_PC32, INSTR_ADDEND);
      addRela(sym, rela);
      addSymUsage(sym.m_name);
      writeEmptyDisp();
    }
  } else {
    addForwardReference(a_sym_name, location_counter-1, RelocationType::R_X86_64_PC32, INSTR_ADDEND);
    addSymUsage(a_sym_name);
    writeEmptyDisp();
  }
}

void handleDirectiveSymbol(const std::string& a_sym_name){
  if(sym_tab.find(a_sym_name) != sym_tab.end() && sym_tab[a_sym_name].m_defined){
    Sym sym = sym_tab[a_sym_name];
    Rela rela = Rela(location_counter, a_sym_name, RelocationType::R_X86_64_32, DIR_ADDEND);
    addRela(sym, rela);
    writeWord(sym_tab[sym.m_sctn_name].m_value + sym.m_value);
  } else {
    addForwardReference(a_sym_name, location_counter, RelocationType::R_X86_64_32, DIR_ADDEND);
    writeWord(0x00000000);
  }
}

/// Expected written first 3 bytes of the instruction
void handleInstructionLiteral(uint32_t a_literal){
  addLiteralUsage(a_literal);
  writeEmptyDisp();
}

void printSymbolTable(){
  std::string label = "SYMTAB";
  std::cout << "**************** " << label << " ****************\n\n";
  for(const auto& kv: sym_tab){
    std::cout<< "sym_name = " << kv.second.m_name << "\n";
    std::cout<< "sym_bind = " << kv.second.m_bind << "\n";
    std::cout<< "sym_type = " << kv.second.m_type << "\n";
    std::cout<< "section_name = " << kv.second.m_sctn_name << "\n";
    std::cout<< "sym_val = " << kv.second.m_value << "\n";
    std::cout<< "sym_defined = " << kv.second.m_defined << "\n";
    std::cout<< "forward_references = \n";
    for(const auto& fr : kv.second.m_forward_ref_table) {
      std::cout << "\tfr_sctn = "<< fr.m_sctn_name<<"\n";
      std::cout << "\tfr_offset = "<< fr.m_offset<<"\n";
      std::cout << "\tfr_reloc_type = "<< fr.m_reloc_type<<"\n";
      std::cout << "\tfr_addend = "<< fr.m_addend<<"\n";
    }
    std::cout<<std::endl;
  }
}

void printRela(){
  std::string label = "RELA SECTION";
  std::cout << "**************** " << label << " ****************\n\n";
  for(const auto& kv: rela_table){
    std::cout<< "section = " << kv.first << "\n\n";
    for (const auto& a_rela : kv.second){
      std::cout<< "offset = " << a_rela.m_offset << "\n";
      std::cout<< "relocation_type = " << a_rela.m_rela_type << "\n";
      std::cout<< "sym_name = " << a_rela.m_sym_name << "\n";
      std::cout<< "addend = " << a_rela.m_addend << "\n\n";
    }
  }
}

void printSections(){
  std::string label = "SECTIONS DATA";
  std::cout << "**************** " << label << " ****************\n\n";
  uint8_t cnt = 0;
  for(const auto& kv : section_data_table){
    cnt = 0;
    std::cout<< "\n#"<<kv.first<<"\n";
    std::cout<< "actual size in bytes = "<<kv.second.size()<<"\n";
    for (const auto& byte : kv.second){
      printf("%.2X", byte);
      if(cnt == 7){
        std::cout<<"\n";
      } else if(cnt == 3){
        std::cout<<"   ";
      } else {
        std::cout<<" ";
      }
      cnt++;
      if(cnt == 8) {
        cnt = 0;
      }
    }
  }
  std::cout<<"\n\n";
}

void printLiteralPool(){
  std::string label = "LITERAL POOL";
  std::cout << "**************** " << label << " ****************\n\n";
  for(const auto&kv : literal_pool){
    std::cout<<"#"<<kv.first<<" size = "<<kv.second.size()<<std::endl;
    for(const auto& literal : kv.second){
        printf("%.2X\n", literal);
    }
  }
  std::cout<<std::endl;
}

void printAll(){
  printSymbolTable();
  printRela();
  printSections();
  printLiteralPool();
}
