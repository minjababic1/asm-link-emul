#include "../inc/common.hpp"
#include "../inc/linker.hpp"
#include "../inc/types.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

SymbolTable linker_sym_tab;
SectionPlaceTable linker_section_place_table;
SectionRelasTable linker_section_relas_table;
SectionDataTable linker_section_data_table;
std::vector<std::string> linker_sections;

const std::size_t SCTN_START_NDX_PLACE_DIR = 7;

void parseSymTabEntry(const std::string& a_line, SymbolTable& a_input_sym_tab) {
  uint32_t num = std::stoul(a_line.substr(SymTabLayout::NUM_OFF, SymTabLayout::NUM_WIDTH));
  uint32_t val = std::stoul(a_line.substr(SymTabLayout::VAL_OFF, SymTabLayout::VAL_WIDTH), nullptr, 16);
  std::size_t type_last_char_off = a_line.find(" ", SymTabLayout::TYPE_OFF);
  SymbolType type = 
    sym_type_to_str_map[a_line.substr(SymTabLayout::TYPE_OFF, type_last_char_off - SymTabLayout::TYPE_OFF)];
  std::size_t bind_last_char_off = a_line.find(" ", SymTabLayout::BIND_OFF);
  SymbolBinding bind = sym_bind_to_str_map[a_line.substr(SymTabLayout::BIND_OFF, bind_last_char_off - SymTabLayout::BIND_OFF)];
  std::size_t sctn_name_last_char_off = a_line.find(" ", SymTabLayout::SCTN_OFF);
  std::string sctn_name = a_line.substr(SymTabLayout::SCTN_OFF, sctn_name_last_char_off - SymTabLayout::SCTN_OFF);
  std::string sym_name = a_line.substr(SymTabLayout::NAME_OFF);
  Sym sym = Sym(sym_name, bind, type, sctn_name, val, sctn_name == UNDEFINED_SCTN ? false : true);
  sym.m_index = num;
  a_input_sym_tab[sym_name] = sym;
}

void parseRelaEntry(
  const std::string& a_line, 
  SectionRelasTable& a_input_section_relas_table, 
  const std::string& a_sctn_name
) {
  uint32_t offset = std::stoul(a_line.substr(RelaLayout::OFFSET_OFF, RelaLayout::OFFSET_WIDTH), nullptr, 16);
  std::size_t type_last_char_off = a_line.find(" ", RelaLayout::TYPE_OFF);
  RelocationType type = 
    rela_type_to_str_map[a_line.substr(RelaLayout::TYPE_OFF, type_last_char_off - RelaLayout::TYPE_OFF)];
  std::size_t sym_name_last_char_off = a_line.find(" ", RelaLayout::SYMBOL_OFF);
  std::string sym_name = a_line.substr(RelaLayout::SYMBOL_OFF, sym_name_last_char_off - RelaLayout::SYMBOL_OFF);
  int32_t addend = std::stoi(a_line.substr(RelaLayout::ADDEND_OFF));
  Rela rela = Rela(offset, sym_name, type, addend);
  a_input_section_relas_table[a_sctn_name].push_back(rela);
}

void parseSectionContentLine(
  const std::string& a_line, 
  SectionDataTable& a_input_section_data_table, 
  const std::string& a_sctn_name
) {
  std::istringstream iss(a_line);
  std::string byte_representation;
  while (iss >> byte_representation) {
      uint8_t byte_val = static_cast<uint8_t>(std::stoul(byte_representation, nullptr, 16));
      a_input_section_data_table[a_sctn_name].push_back(byte_val);
  }
}

int8_t parseLinkerInput(
  const std::string& a_input_file, 
  SymbolTable& a_input_sym_tab,
  SectionRelasTable& a_input_section_relas_table,
  SectionDataTable& a_input_section_data_table,
  std::vector<std::string>& a_input_sections
) {
  std::ifstream in(a_input_file);

  if (!in.is_open()) {
      std::cerr << "Greska prilikom otvaranja fajla: " << a_input_file << "\n";
      return 1;
  }

  std::string line;
  std::getline(in, line); /// #.symtab
  std::getline(in, line); /// symtab header

  while (std::getline(in, line) && line[0] != '#') {
      parseSymTabEntry(line, a_input_sym_tab);
  }

  while(true) {
    if (line.find(RELA_SCTN_PREFIX) != std::string::npos) {
      std::string sctn_name = line.substr(RELA_SCTN_NAME_OFF);
      std::getline(in, line); /// rela header
      while (std::getline(in, line) && !line.empty() && line[0] != '#') {
        parseRelaEntry(line, a_input_section_relas_table, sctn_name);
      }
    } else if (!line.empty() && line[0] == '#'){
      std::string sctn_name = line.substr(SCTN_NAME_OFF);
      a_input_sections.push_back(sctn_name);
      while (std::getline(in, line) && !line.empty() && line[0] != '#') {
        parseSectionContentLine(line, a_input_section_data_table, sctn_name);
      }
    } else {
      break;
    }
  }

  in.close();
  return 0;
}

bool sortAndValidatePlaceSections() {
  std::sort(
    linker_section_place_table.begin(), 
    linker_section_place_table.end(), 
    [] (const auto& a_higher, const auto& a_lower) {
      return a_higher.m_addr < a_lower.m_addr;
    }
  );

  for (std::size_t i = 0; i < linker_section_place_table.size(); i++) {
    if (i != linker_section_place_table.size() - 1) {
      std::uint32_t sctn_size = linker_section_data_table[linker_section_place_table[i].m_sctn_name].size();
      if (linker_section_place_table[i].m_addr + sctn_size > linker_section_place_table[i+1].m_addr) {
        std::cerr << "Greska: Preklapanje sekcija " << linker_section_place_table[i].m_sctn_name 
          << " i " << linker_section_place_table[i+1].m_sctn_name << " zbog -place opcije\n";
        return false;
      } else {
        if (linker_section_place_table[i].m_addr + sctn_size > 0xFFFFFF00) {
          std::cerr << "Greska: Sekcija " << linker_section_place_table[i].m_sctn_name 
              << "ne moze da stane na adresu zadatu -place opcijom\n";
            return false;
        }
      }
    }
  }
  return true;
}

bool hasConflictingSymbolDefinitions(
  SymbolTable& a_input_sym_tab,
  SymbolTable& a_existing_sym_tab
) {
  for(const auto& [sym_name, sym] : a_input_sym_tab) {
    if (a_existing_sym_tab.find(sym_name) != a_existing_sym_tab.end() && 
        a_existing_sym_tab[sym_name].m_type != sym.m_type
    ) {
      std::cerr << "Greska: Vise puta se koristi simbol " << sym_name << " sa razlicitm tipom" << std::endl;
      return true;
    }
    if (a_existing_sym_tab.find(sym_name) != a_existing_sym_tab.end() && 
        a_existing_sym_tab[sym_name].m_bind != sym.m_bind
    ) {
      std::cerr << "Greska: Vise puta se koristi simbol " << sym_name << " sa razlicitm vezivanjem" << std::endl;
      return true;
    }
    if (sym.m_defined && 
        a_existing_sym_tab.find(sym_name) != a_existing_sym_tab.end() && 
        a_existing_sym_tab[sym_name].m_defined &&
        sym.m_type != SymbolType::SCTN) {
      std::cerr << "Greska: Visestruka definicija simbola " << sym_name << std::endl;
      return true;
    }
  }
  return false;
}

bool hasOverlappingSection(
  const std::string& a_input_section,
  std::vector<std::string>& a_existing_sections
) {
  for (const auto& existing_section : a_existing_sections) {
    if (a_input_section == existing_section) {
      return true;
    }
  }
  return false;
}

void handleSectionsOverlapping(
  const std::string& a_input_section,
  SymbolTable& a_input_sym_tab,
  SectionRelasTable& a_input_section_relas_table
) {
  uint32_t existing_sctn_sz = linker_section_data_table[a_input_section].size();
  
  /// update value of the symbols defined in the overlapping section
  for (auto& [sym_name, sym] : a_input_sym_tab) {
    if (sym.m_sctn_name == a_input_section && sym.m_type != SymbolType::SCTN) {
      sym.m_value+= existing_sctn_sz;
    }
  }

  /// update offset of the overlapping section relocations
  for (auto& rela : a_input_section_relas_table[a_input_section]) {
    rela.m_offset+= existing_sctn_sz;
  }
}

void mergeSymbolTables(
  SymbolTable& a_input_sym_tab,
  SymbolTable& a_existing_sym_tab
) {
  for (const auto& [sym_name, sym] : a_input_sym_tab) {
    if (sym.m_bind == SymbolBinding::LOC &&  sym.m_type != SymbolType::SCTN) {
      continue;
    }
    if (a_existing_sym_tab.find(sym_name) != a_existing_sym_tab.end() &&
        a_existing_sym_tab[sym_name].m_type == SymbolType::SCTN
    ) {
      continue;
    }
    if (a_existing_sym_tab.find(sym_name) == a_existing_sym_tab.end() || 
        (!a_existing_sym_tab[sym_name].m_defined && sym.m_defined)
    ) {
        a_existing_sym_tab[sym_name] = sym;
    }
  }
}

void mergeRelocations(
  SectionRelasTable& a_input_section_relas_table, 
  SectionRelasTable& a_existing_section_relas_table
) {
  for (const auto& [section, relocations] : a_input_section_relas_table) {
    for (const auto& rela : relocations) {
      a_existing_section_relas_table[section].push_back(rela);
    }
  }
}

void mergeSectionContents(
  SectionDataTable& a_input_section_data_table, 
  SectionDataTable& a_existing_section_data_table
) {
  for (const auto& [section, data] : a_input_section_data_table) {
    for (const auto& byte : data) {
      a_existing_section_data_table[section].push_back(byte);
    }
  }
}

int8_t handleInputFile(const std::string& a_input_file) {
  SymbolTable input_sym_tab;
  SectionRelasTable input_section_relas_table;
  SectionDataTable input_section_data_table;
  std::vector<std::string> input_sections;
  if (parseLinkerInput(
        a_input_file,
        input_sym_tab,
        input_section_relas_table,
        input_section_data_table,
        input_sections) == 1) {
          return 1;
  }
  
  if (hasConflictingSymbolDefinitions(input_sym_tab, linker_sym_tab)) {
    return 1;
  }

  for (const auto& input_section : input_sections) {
    bool is_overlapping_section = hasOverlappingSection(input_section, linker_sections);
    if (is_overlapping_section) {
      handleSectionsOverlapping(input_section, input_sym_tab, input_section_relas_table);
    } else {
      linker_sections.push_back(input_section);
    }
  }



  mergeSymbolTables(input_sym_tab, linker_sym_tab);
  mergeRelocations(input_section_relas_table, linker_section_relas_table);
  mergeSectionContents(input_section_data_table, linker_section_data_table);

  return 0;
}

bool hasUndefinedSymbols(SymbolTable& a_sym_tab) {
  for (const auto& [sym_name, sym] : a_sym_tab) {
    if (!sym.m_defined) {
      std::cerr << "Greska: Ne postoji definicija simbola " << sym_name << std::endl;
      return true;
    }
  }
  return false;
}

void handleArguments(
  int argc,
  char* argv[],
  std::vector<std::string>& input_files,
  std::string& output_file,
  bool& hex_mode
) {
  for(uint8_t i = 1; i < argc; i++) {
    std::string arg = std::string(argv[i]);
    if (arg == "-o") {
      output_file = std::string(argv[i+1]);
      ++i;
    } else if (arg.find("-place=") != std::string::npos) {
      std::size_t delimeter_pos = arg.find("@");
      std::string scnt_name = 
        arg.substr(SCTN_START_NDX_PLACE_DIR, delimeter_pos - SCTN_START_NDX_PLACE_DIR);
      uint32_t addr = 
        static_cast<uint32_t>(std::stoul(arg.substr(delimeter_pos + 1), nullptr, 0));
      linker_section_place_table.push_back(SectionPlace(scnt_name, addr));
    } else if (arg == "-hex") {
      hex_mode = true;
    } else {
      input_files.push_back(arg);
    }
  }

  
}

uint32_t alignedAddr(uint32_t a_addr) {
  return (a_addr + 15) & ~0xF;
}

void linkSectionToAddr() {
  uint32_t location_counter = 0x00000000;
  std::unordered_set<std::string> placed_sections;

  for (const auto& section_place_entry : linker_section_place_table) {
    linker_sym_tab[section_place_entry.m_sctn_name].m_value = section_place_entry.m_addr;
    placed_sections.insert(section_place_entry.m_sctn_name);
    location_counter =
       section_place_entry.m_addr + linker_section_data_table[section_place_entry.m_sctn_name].size();
    location_counter = alignedAddr(location_counter);
  }

  for (const auto& section : linker_sections) {
    if (placed_sections.find(section) == placed_sections.end()) {
      linker_sym_tab[section].m_value = location_counter;
      location_counter+= linker_section_data_table[section].size();
      location_counter = alignedAddr(location_counter);
      placed_sections.insert(section);
    }
  }

  std::sort(
    linker_sections.begin(), 
    linker_sections.end(),
    [] (const auto& a_higher, const auto& a_lower) {
      return linker_sym_tab[a_higher].m_value < linker_sym_tab[a_lower].m_value;
    }
  );
}

void updateSymTab() {
  for (auto& [sym_name, sym] : linker_sym_tab) {
    if (sym.m_type != SymbolType::SCTN) {
      sym.m_value+= linker_sym_tab[sym.m_sctn_name].m_value;
    }
  }
}

void applyRelocations() { 
  for (const auto& section : linker_sections) {
    for (const auto& rela : linker_section_relas_table[section]) {
      updateWord(
        linker_section_data_table, 
        section, 
        rela.m_offset, 
        linker_sym_tab[rela.m_sym_name].m_value + rela.m_addend
      );
    }
  }
}

int main(int argc, char* argv[]) {
  std::vector<std::string> input_files;
  std::string output_file = "build/program.hex";
  bool hex_mode = false;
  handleArguments(argc, argv, input_files, output_file, hex_mode);

  if(!hex_mode) {
    std::cerr << "Greska: Nije prosledjena opcija u kom modu linker treba da radi" << std::endl;
    return 1;
  }

  for(const auto& input_file : input_files) {
    if (handleInputFile(input_file) == 1) {
      return 1;
    }
  }

  if (hasUndefinedSymbols(linker_sym_tab)) {
    return 1;
  }

  if (!sortAndValidatePlaceSections()) {
    return 1;
  }

  linkSectionToAddr();
  updateSymTab();
  applyRelocations();

  writeSymTab(std::cout, linker_sym_tab);
  // writeRela(std::cout, linker_section_relas_table, linker_sections);
  writeSections(std::cout, linker_section_data_table, linker_sections, linker_sym_tab, true);
}
