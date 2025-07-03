#include "../inc/common.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>

ElfHdr elf_hdr;
std::unordered_map<std::string, SectionHdr> section_headers;

namespace SymTabLayout{
  const std::size_t NUM_OFF = 0;
  const std::size_t NUM_WIDTH = 6;

  const std::size_t VAL_OFF = 6;
  const std::size_t VAL_WIDTH = 8;

  const std::size_t SZ_OFF = 14;
  const std::size_t SZ_WIDTH = 6;

  const std::size_t TYPE_OFF = 22;
  const std::size_t TYPE_WIDTH = 7;

  const std::size_t BIND_OFF = 29;
  const std::size_t BIND_WIDTH = 6;

  const std::size_t SCTN_OFF = 35;
  const std::size_t SCTN_WIDTH = 20;

  const std::size_t NAME_OFF = 55;
}

namespace RelaLayout{
  const std::size_t OFFSET_OFF = 0;
  const std::size_t OFFSET_WIDTH = 8;

  const std::size_t TYPE_OFF = 10;
  const std::size_t TYPE_WIDTH = 13;

  const std::size_t SYMBOL_OFF = 23;
  const std::size_t SYMBOL_WIDTH = 12;

  const std::size_t ADDEND_OFF = 35;
  const std::size_t ADDEND_WIDTH = 6;
}; // namespace

const std::string UNDEFINED_SCTN = "UND";
const std::string RELA_SCTN_PREFIX = "#.rela.";
const std::size_t RELA_SCTN_NAME_OFF = 7;
const std::size_t SCTN_NAME_OFF = 2;

std::unordered_map<std::string, SymbolBinding> sym_bind_to_str_map = {
  {"LOC", SymbolBinding::LOC},
  {"GLOB", SymbolBinding::GLOB}
};

std::unordered_map<std::string, SymbolType> sym_type_to_str_map = {
  {"NOTYP", SymbolType::NOTYP},
  {"SCTN", SymbolType::SCTN},
  {"OBJ", SymbolType::OBJ}
};

std::unordered_map<std::string, RelocationType> rela_type_to_str_map = {
  {"R_X86_64_32", RelocationType::R_X86_64_32}
};

std::ostream& operator<<(std::ostream& os, SymbolBinding binding) {
  switch(binding) {
    case LOC: os << "LOC"; break;
    case GLOB: os << "GLOB"; break;
    default: os << "UNDEF"; break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, SymbolType type) {
  switch(type) {
    case NOTYP: os << "NOTYP"; break;
    case SCTN: os << "SCTN"; break;
    case OBJ: os << "OBJ"; break;
    default: os << "UNDEF"; break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, RelocationType reloc) {
  switch(reloc) {
    case R_X86_64_32: os << "R_X86_64_32"; break;
    default: os << "UNDEF"; break;
  }
  return os;
}

void updateByte(
  SectionDataTable& a_section_data_table, 
  const std::string& a_sctn_name, 
  uint32_t a_offset, 
  uint8_t a_byte
){
  a_section_data_table[a_sctn_name][a_offset] = a_byte;
}

void updateWord(
  SectionDataTable& a_section_data_table, 
  const std::string& a_sctn_name, 
  uint32_t a_offset,
  uint32_t a_word
) {
  updateByte(
    a_section_data_table, 
    a_sctn_name,
     a_offset, 
     static_cast<uint8_t>(a_word & 0xFF)
  );
  updateByte(
    a_section_data_table, 
    a_sctn_name, 
    a_offset + 1, 
    static_cast<uint8_t>((a_word >> 8) & 0xFF)
  );
  updateByte(
    a_section_data_table, 
    a_sctn_name,
    a_offset + 2, 
    static_cast<uint8_t>((a_word >> 16) & 0xFF)
  );
  updateByte(
    a_section_data_table, 
    a_sctn_name, 
    a_offset + 3, 
    static_cast<uint8_t>((a_word >> 24) & 0xFF)
  );
}

void writeSymTab(std::ostream& a_out, SymbolTable& a_sym_tab){  
  a_out << "#.symtab\n";
  a_out << std::left
          << std::setw(6)  << "Num"
          << std::setw(10) << "Value"
          << std::setw(4)  << "Size"
          << std::setw(9)  << "  Type"
          << std::setw(6)  << "Bind"
          << std::setw(20) << "Sctn"
          << std::setw(4) << "Name"
          << "\n";

  SymbolList sorted_syms;
  sorted_syms.reserve(a_sym_tab.size());

  for (const auto& [name, sym] : a_sym_tab) {
      sorted_syms.emplace_back(sym);
  }

  std::sort(sorted_syms.begin(), sorted_syms.end(),
            [](const auto& a_higher, const auto& a_lower) {
              if(a_higher.m_type == SymbolType::SCTN && a_lower.m_type != SymbolType::SCTN){
                return true;
              } else if (a_higher.m_type != SymbolType::SCTN && a_lower.m_type == SymbolType::SCTN){
                return false;
              }
              return a_higher.m_index < a_lower.m_index;
            });
  uint32_t sym_tab_cnt = 0;
  for(auto& sym: sorted_syms){
    sym.m_index = sym_tab_cnt++;
     a_out << std::left
              << std::setw(SymTabLayout::NUM_WIDTH) << sym.m_index
              << std::setw(SymTabLayout::VAL_WIDTH) << std::hex << std::right << std::setfill('0') 
              << std::uppercase << sym.m_value << std::dec << std::setfill(' ')
              << std::setw(SymTabLayout::SZ_WIDTH) << 0 << std::left
              << "  "
              << std::setw(SymTabLayout::TYPE_WIDTH) << sym.m_type 
              << std::setw(SymTabLayout::BIND_WIDTH) << sym.m_bind
              << std::setw(SymTabLayout::SCTN_WIDTH) << sym.m_sctn_name
              << sym.m_name
              << "\n";
  }
}

void writeRela(
  std::ostream& a_out, 
  SectionRelasTable& a_section_relas_table, 
  std::vector<std::string>& a_sections
) {
  for(const auto& section: a_sections){
    if(a_section_relas_table.find(section) == a_section_relas_table.end()){
      continue;
    }
    a_out<< "#.rela." << section << "\n";
    a_out << std::left
              << std::setw(10)  << "Offset"
              << std::setw(13)  << "Type"
              << std::setw(12) << "Symbol"
              << std::setw(8)  << "Addend"
              << "\n";

    std::sort(a_section_relas_table[section].begin(), a_section_relas_table[section].end(), 
      [](const auto& a_left, const auto& a_right){
        return a_left.m_offset < a_right.m_offset;
    });
    for (const auto& rela : a_section_relas_table[section]){
      a_out << std::left
                << std::setw(RelaLayout::OFFSET_WIDTH)  << std::hex << std::right << std::setfill('0') << 
                  rela.m_offset << std::dec << std::left << std::setfill(' ')
                << "  "
                << std::setw(RelaLayout::TYPE_WIDTH)  << rela.m_rela_type
                << std::setw(RelaLayout::SYMBOL_WIDTH) << rela.m_sym_name
                << std::setw(RelaLayout::ADDEND_WIDTH)  << std::right << rela.m_addend << std::left
                << "\n";
    }
  }
}

void writeSections(
  std::ostream& a_out, 
  SectionDataTable& a_section_data_table,
  std::vector<std::string>& a_sections,
  SymbolTable& a_sym_tab,
  bool a_hex_mode
) {
  for (const auto& section : a_sections) {
    const auto addr = a_sym_tab[section].m_value;
    const auto& data = a_section_data_table[section];
    
    if (!a_hex_mode) {
      a_out << "#." << section << "\n" << std::right;
    }  

    size_t i = 0;
    for (; i < data.size(); i++) {
      if (i % 8 == 0 && a_hex_mode) {
        a_out 
          << std::hex << std::setw(8) << std::setfill('0') << (addr + i) 
          << std::dec << std::setfill(' ') << ": ";
      }
      a_out << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
            << static_cast<uint16_t>(data[i] & 0x00FF) << std::dec << std::setfill(' ');

      if (i % 8 == 7) {
        a_out << "\n";
      } else if (i % 8 == 3) {
        a_out << "   ";
      } else {
        a_out << " ";
      }
    }

    if (a_hex_mode) {
      for (; i % 8 != 0 ; i++) {
        a_out << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
            << 0x00 << std::dec << std::setfill(' ');

        if (i % 8 == 7) {
          a_out << "\n";
        } else if (i % 8 == 3) {
          a_out << "   ";
        } else {
          a_out << " ";
        }
      }
    } else if (data.size() % 8 != 0) {
      a_out << std::endl;
    }
  }
}
