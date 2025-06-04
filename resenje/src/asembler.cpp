#include "../inc/asembler.hpp"
#include <iostream>

std::vector<std::string> global_symbols;
std::vector<std::string> extern_symbols;
std::string current_directive;

void insertIntoCurrentDirectiveList(const char* symbol) {
    std::string sym(symbol);
    if (current_directive == "global") {
        global_symbols.push_back(sym);
    } else if (current_directive == "extern") {
        extern_symbols.push_back(sym);
    }
}

// void insertIntoSymbolTable(std::string a_name, Symbol a_symbol){
//   symbol_table[a_name] = a_symbol;
// }

// void insertIntoRelocationTable(std::string a_name, Relocation a_relocation){
//   relocation_table[a_name] = a_relocation;
// }

void printSymbolLists() {
    printf("Global symbols:\n");
    for (const auto& sym : global_symbols) {
        std::cout<< sym << " ";
    }

    printf("\nExtern symbols:\n");
    for (const auto& sym : extern_symbols) {
        std::cout<< sym << " ";
    }
}
