#include "../inc/asembler.hpp"
#include <iostream>

std::unordered_map<std::string, Sym> sym_tab;
std::unordered_map<std::string, std::vector<Rela>> rela_table;
std::unordered_map<std::string, std::vector<uint8_t>> section_data_table;
std::unordered_map<uint32_t, std::vector<uint32_t>> literal_usage_table;
std::unordered_map<std::string, std::vector<uint32_t>> literal_pool;
std::unordered_set<std::string> extern_symbols;
std::unordered_set<std::string> global_symbols;

std::string current_section = "";
uint32_t location_counter = 0;
uint32_t total_offset = 0;
const std::string UNDEFINED_SCTN = "UND";

void addSym(Sym a_sym){
  if( sym_tab.find(a_sym.m_name) == sym_tab.end()) {
    sym_tab[a_sym.m_name] = a_sym;
  }
}

void addRela(Rela a_rela){
  rela_table[current_section].push_back(a_rela);
}

void openNewSection(std::string a_sctn_name){
  current_section = a_sctn_name;
  location_counter = 0;
  literal_usage_table.clear();
}

void closeCurrentSection(){
  // jump over literal pool
  if(literal_usage_table.size() > 0){
    section_data_table[current_section].push_back(0x30);
    section_data_table[current_section].push_back(0xF0);
    uint16_t disp = literal_usage_table.size()*4;
    section_data_table[current_section].push_back((static_cast<uint8_t>(disp >> 8)) & 0x0F);
    section_data_table[current_section].push_back(static_cast<uint8_t>(disp & 0x00FF));
    adjustLocation(4);
  }

  // make usage of literal point to literal in the pool
  for(const auto& kv : literal_usage_table){
    writeWord(kv.first);
    for(const auto& usage_addr : kv.second){
      uint16_t disp = location_counter - usage_addr - 2;
      section_data_table[current_section][usage_addr] |= static_cast<uint8_t>((disp >> 8) & 0x0F);
      section_data_table[current_section][usage_addr+1] |= static_cast<uint8_t>(disp & 0x00FF);
    }
    adjustLocation(4);
    literal_pool[current_section].push_back(kv.first);
  }
}

void writeByte(uint8_t a_byte){
  section_data_table[current_section].push_back(a_byte);
}

void writeWord(uint32_t a_word){
  writeByte(static_cast<uint8_t>(a_word & 0xFF));
  writeByte(static_cast<uint8_t>((a_word >> 8) & 0xFF));
  writeByte(static_cast<uint8_t>((a_word >> 16) & 0xFF));
  writeByte(static_cast<uint8_t>((a_word >> 24) & 0xFF));
}

void adjustLocation(uint32_t a_bytes){
  location_counter+= a_bytes;
  total_offset+= a_bytes;
}

void reportSymUsage(const std::string& a_sym_name, RelocationType a_rela_type, uint8_t a_reg_c, int32_t a_addend){
  if(sym_tab.find(a_sym_name) != sym_tab.end() && sym_tab[a_sym_name].m_defined){
    Sym sym = sym_tab[a_sym_name];
    if(sym.m_sctn_name == current_section && a_rela_type == RelocationType::R_X86_64_PC32) {
      uint16_t disp = sym.m_value - location_counter - a_addend;
      writeByte((a_reg_c << 4) | (static_cast<uint8_t>(disp >> 8) & 0x0F));
      writeByte(static_cast<uint8_t>(disp & 0x00FF));
      adjustLocation(2);
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
        adjustLocation(2);
        return;
      } else{
        writeWord(sym.m_value);
        adjustLocation(4);
        return;
      }
    }
  } else{
    Sym sym;
    if(sym_tab.find(a_sym_name) == sym_tab.end()){
      sym = Sym(a_sym_name);
      addSym(sym);
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
        adjustLocation(2);
        return;
      } else{
        writeWord(0x00000000);
        adjustLocation(4);
        return;
    }
  }
}


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

int8_t backPatch(){
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

void reportLiteralUsage(uint32_t a_literal, uint8_t a_reg_c){
  a_reg_c &= 0x0F;
  literal_usage_table[a_literal].push_back(location_counter);
  section_data_table[current_section].push_back((a_reg_c << 4));
  section_data_table[current_section].push_back(0x00);
  adjustLocation(2);
}

void reportGlobalSym(const std::string& a_sym_name){
  global_symbols.insert(a_sym_name);
}
void reportExternSym(const std::string& a_sym_name){
  extern_symbols.insert(a_sym_name);
}

void defineSym(const std::string& a_sym_name){
  if(sym_tab.find(a_sym_name) == sym_tab.end()){
    addSym(Sym(a_sym_name));
  }
  Sym sym = sym_tab[a_sym_name];
  sym.m_sctn_name = current_section;
  sym.m_value = location_counter;
  sym.m_defined = true;
  sym_tab[a_sym_name] = sym;
}

void writeInstr(uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b,
  uint8_t a_reg_c,
  uint16_t a_disp) {
    writeFirstTwoBytesOfTheInstr(a_oc, a_mod, a_reg_a, a_reg_b);
    writeByte((a_reg_c << 4) | ((static_cast<uint8_t>(a_disp >> 8)) & 0x0F));
    writeByte(a_disp & 0x00FF);
    adjustLocation(2);
  }

void writeFirstTwoBytesOfTheInstr(uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b){
    writeByte((a_oc << 4) | a_mod);
    writeByte((a_reg_a << 4) | a_reg_b);
    adjustLocation(2);
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
