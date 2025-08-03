#include "../inc/asembler.hpp"
#include "../inc/instructions.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

SymbolTable sym_tab;
SectionRelasTable section_relas_table;
SectionDataTable section_data_table;
LiteralUsagesTable literal_usages_table;
SectionLiteralsTable literal_pool;
SymbolUsagesTable symbol_usages_table;
SectionSymbolsTable symbol_pool;
std::vector<std::string> sections;

std::string current_section = "";
uint32_t location_counter = 0;
uint32_t total_offset = 0;
const uint8_t INSTR_SIZE = 4;
const uint8_t INSTR_ADDEND = 2;
const uint8_t DIR_ADDEND = 0;
uint32_t defined_sym_cnt = 0;

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

uint8_t readByte(const std::string& a_sctn_name, uint32_t a_addr) {
  return section_data_table[a_sctn_name][a_addr];
}

uint32_t readWord(const std::string& a_sctn_name, uint32_t a_addr) {
  return static_cast<uint32_t>(readByte(a_sctn_name, a_addr)) |
    (static_cast<uint32_t>(readByte(a_sctn_name, a_addr + 1)) << 8) |
    (static_cast<uint32_t>(readByte(a_sctn_name, a_addr + 2)) << 16) |
    (static_cast<uint32_t>(readByte(a_sctn_name, a_addr + 3)) << 24);
}

uint32_t readInstr(const std::string& a_sctn_name, uint32_t a_addr) {
  return (static_cast<uint32_t>(readByte(a_sctn_name, a_addr)) << 24) |
    (static_cast<uint32_t>(readByte(a_sctn_name, a_addr + 1)) << 16) |
    (static_cast<uint32_t>(readByte(a_sctn_name, a_addr + 2)) << 8) |
    static_cast<uint32_t>(readByte(a_sctn_name, a_addr + 3));
}

/**
 * @brief Returns wheter symbol is defined in symbol table or not
 * 
 * @param a_sym_name Name of the given symbol
 */
bool symbolDefined(const std::string& a_sym_name) {
  return sym_tab.find(a_sym_name) != sym_tab.end() && sym_tab[a_sym_name].m_defined;
}

void insertSymbolIfAbsent(Sym a_sym){
  if( sym_tab.find(a_sym.m_name) == sym_tab.end()) {
    sym_tab[a_sym.m_name] = a_sym;
  }
}

/**
 * @brief Adds a relocation entry to the relocation table for the current section
 * 
 * @param a_sym Symbol associated with the relocation
 * @param a_rela Relocation entry to be added
 */
void addRela(Sym a_sym, Rela& a_rela, const std::string& a_sctn_name){
  if(a_sym.m_bind == SymbolBinding::LOC){
    a_rela.m_sym_name = a_sym.m_sctn_name;
    a_rela.m_addend+= a_sym.m_value;
  }
  section_relas_table[a_sctn_name].push_back(a_rela);
}

/**
 * @brief Adds usage of the symbol inside instruction for the current section
 * 
 * @param a_sym_name Name of the given symbol
 */
void addSymUsage(const std::string a_sym_name){
  insertSymbolIfAbsent(Sym(a_sym_name));
  symbol_usages_table[a_sym_name].push_back(location_counter-INSTR_ADDEND);
}

/**
 * @brief Adds usage of the literal inside instruction for the current section
 * 
 * @param a_sym_name Value of the given literal
 */
void addLiteralUsage(uint32_t a_literal){
  literal_usages_table[a_literal].push_back(location_counter-INSTR_ADDEND);
}

/**
 * @brief Adds forward reference to the symbol for the current section
 * 
 * @param a_sym_name Name of the given symbol
 * @param a_offset Offset within the section where the symbol is referenced
 * @param a_addend Represents an additional information used for the relocation
 */
void addForwardReference(const std::string a_sym_name,
  uint32_t a_offset,
  int32_t a_addend
){
  insertSymbolIfAbsent(Sym(a_sym_name));
  Sym sym = sym_tab[a_sym_name];
  sym.m_forward_ref_table.push_back(
    ForwardReferenceEntry(
      current_section,
      a_offset,
      a_addend
    )
  );
  sym_tab[a_sym_name] = sym;
}

void openNewSection(std::string a_sctn_name){
  current_section = a_sctn_name;
  sections.push_back(a_sctn_name);
  location_counter = 0;
  literal_usages_table.clear();
  symbol_usages_table.clear();
}

/**
 * @brief Patches the third and fourth byte of the instruction to complete 12-bit displacement
 * 
 * @param a_disp Displacement value
 * @param a_offset Start address of the disp field
 **/ 
void patchDispField(const std::string a_sctn_name, uint32_t a_offset, uint16_t a_disp) {
    uint8_t regC_bits = section_data_table[a_sctn_name][a_offset] & 0xF0;
    uint8_t disp_high = static_cast<uint8_t>((a_disp >> 8) & 0x0F);
    uint8_t disp_low = static_cast<uint8_t>(a_disp & 0x00FF);

    updateByte(section_data_table, a_sctn_name, a_offset, regC_bits | disp_high);
    updateByte(section_data_table, a_sctn_name, a_offset + 1, disp_low);
}

/**
 * @brief Patches the mod field of the instruction when symbol is defined in the same section
 * 
 * @param a_instr_addr Address of the instruction
 */
void patchModField(uint32_t a_instr_addr) {
  uint32_t instr = readInstr(current_section, a_instr_addr);
  int oc = static_cast<int>((instr >> 28) & 0xF);
  uint8_t mod_val = static_cast<uint8_t>((instr >> 24) & 0xF); 
  switch (oc) {
    case OpCode::CALL:
    case OpCode::LD:
      mod_val-= 1;
      break;
    case OpCode::ST:
      mod_val-= 2;
      break;
    case OpCode::JMP:
      mod_val-= 4;
      break;
    default:
      return;
  }
  updateByte(section_data_table, current_section, a_instr_addr, (oc << 4) | (mod_val & 0x0F));
}

void closeCurrentSection(){
  uint32_t symbol_pool_size = 0;
  for(const auto& [sym_name, usages] : symbol_usages_table){
    if(!symbolDefined(sym_name)){
      symbol_pool_size++;
    }
  }

  // jump over literal and symbol pool
  if(literal_usages_table.size() > 0 || symbol_pool_size > 0){
    writeInstruction(0x03, 0x00, 0x0F, 0x00, 0x00, (literal_usages_table.size()+symbol_pool_size)*4);
  }

  // make usage of literal point to literal in the pool
  for(const auto& [literal, usages] : literal_usages_table){
    for(const auto& usage_addr : usages){
      uint16_t disp = location_counter - usage_addr - INSTR_ADDEND;
      patchDispField(current_section, usage_addr, disp);
    }
    writeWord(literal);
    literal_pool[current_section].push_back(literal);
  }

  // make usage of the symbol point to the symbol in the pool
  for(const auto& [sym_name, usages] : symbol_usages_table){
    bool defined_after_usage = symbolDefined(sym_name);
    for(const auto& usage_addr : usages){
      uint16_t disp;
      if (defined_after_usage) {
        disp = sym_tab[sym_name].m_value - usage_addr - INSTR_ADDEND;
        patchModField(usage_addr - 2);
      } else {
        disp = location_counter - usage_addr - INSTR_ADDEND;;
      }
      patchDispField(current_section, usage_addr, disp);
    }
    if(!defined_after_usage){
      addForwardReference(sym_name, location_counter, DIR_ADDEND);
      writeWord(0x00000000);
      symbol_pool[current_section].push_back(sym_name);
    }
  }
}

int8_t applyBackpatching(){
  for(const auto& [sym_name, sym] : sym_tab){
    if(!symbolDefined(sym.m_name) && sym.m_sctn_name != UNDEFINED_SCTN){
      return 1;
    }
    for(const auto& forward_ref : sym.m_forward_ref_table){
      Rela rela = Rela(forward_ref.m_offset, sym.m_name, RelocationType::R_X86_64_32, forward_ref.m_addend);
        addRela(sym, rela, forward_ref.m_sctn_name);
    }
  }
  return 0;
}

void defineSymbol(const std::string& a_sym_name, SymbolType a_type){
  insertSymbolIfAbsent(Sym(a_sym_name));
  Sym sym = sym_tab[a_sym_name];
  sym.m_type = a_type;
  sym.m_sctn_name = current_section;
  sym.m_value = location_counter;
  sym.m_defined = true;
  sym.m_index = defined_sym_cnt++;
  sym_tab[a_sym_name] = sym;
}

void writeInstruction(
  uint8_t a_oc, 
  uint8_t a_mod, 
  uint8_t a_reg_a, 
  uint8_t a_reg_b,
  uint8_t a_reg_c,
  uint16_t a_disp
) {
    writeInstructionFixedFields(a_oc, a_mod, a_reg_a, a_reg_b, a_reg_c);
    patchDispField(current_section, location_counter - INSTR_ADDEND, a_disp);
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
    writeByte(0x00);
}

/// Called after entire instruction is written with disp = 0
void handleInstructionSymbol(const std::string& a_sym_name){
  if(symbolDefined(a_sym_name) && sym_tab[a_sym_name].m_sctn_name == current_section){
    uint16_t disp = sym_tab[a_sym_name].m_value - location_counter;
    patchDispField(current_section, location_counter - INSTR_ADDEND, disp);
    patchModField(location_counter - INSTR_SIZE);
  } else {
    addSymUsage(a_sym_name);
  }
}

void handleDirectiveSymbol(const std::string& a_sym_name){
  if(symbolDefined(a_sym_name)){
    Sym sym = sym_tab[a_sym_name];
    Rela rela = Rela(location_counter, a_sym_name, RelocationType::R_X86_64_32, DIR_ADDEND);
    addRela(sym, rela, current_section);
  } else {
    addForwardReference(a_sym_name, location_counter, DIR_ADDEND);
  }
  writeWord(0x00000000);
}

/// Called after entire instruction is written with disp = 0,
void handleInstructionLiteral(uint32_t a_literal){
  addLiteralUsage(a_literal);
}

void writeLiteralPool(std::ofstream& a_out){
  if(literal_pool.size() == 0){
    return;
  }

  std::string label = "LITERAL POOL";
  a_out << "**************** " << label << " ****************\n\n";
  for(const auto& section : sections){
    if(literal_pool[section].size() == 0){
      continue;
    }
    a_out<<"#"<<section<<" size = "<<literal_pool[section].size()<<std::endl;
    for(auto& literal : literal_pool[section]){
        a_out << std::right << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << literal << std::dec << std::setfill(' ') << "\n";
    }
  }
  a_out<<"\n\n";
}

void writeSymbolPool(std::ofstream& a_out){
  if(symbol_pool.size() == 0){
    return;
  } 

  std::string label = "SYMBOL POOL";
  a_out << "**************** " << label << " ****************\n\n";
  for(const auto& section : sections){
    if(symbol_pool[section].size() == 0){
      continue;
    }
    a_out<<"#"<<section<<" size = "<<symbol_pool[section].size()<<std::endl;
    for(const auto& sym_name : symbol_pool[section]){
        a_out<<sym_name<<std::endl;
    }
  }
  a_out<<"\n\n";
}

void writeObj(std::ofstream& a_out){
  writeSymTab(a_out, sym_tab);
  writeRela(a_out, section_relas_table, sections);
  writeSections(a_out, section_data_table, sections, sym_tab, false);
}

extern int yyparse();

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file = "build/out.o";

    if (argc == 2) {
        input_file = argv[1];
    }
    else if (argc == 4) {
        if (std::string(argv[1]) == "-o") {
            output_file = argv[2];
            input_file = argv[3];
        } else if (std::string(argv[2]) == "-o") {
            input_file = argv[1];
            output_file = argv[3];
        } else {
            std::cerr << "Greška: neispravna upotreba opcije -o\n";
            return 1;
        }
    }
    else {
        std::cerr << "Greška: neispravan format argumenata.\n";
        return 1;
    }

    FILE* input = fopen(input_file.c_str(), "r");
      if (!input) {
      std::cerr << "Greška: ne mogu da otvorim ulaznu datoteku " << input_file << "\n";
      return 1;
      }

    extern FILE* yyin;
    yyin = input;
    yyparse();
    if(applyBackpatching() == 1){
      std::cerr << "Greška: Postoji simbol koji nije eksterni i nije definisan " << output_file << "\n";
      return 1;
    }

    std::ofstream out(output_file);
    if (!out) {
        std::cerr << "Greška: ne mogu da otvorim izlaznu datoteku " << output_file << "\n";
        return 1;
    }
  
    writeObj(out);

    out.close();
    fclose(yyin);

    return 0;
}

