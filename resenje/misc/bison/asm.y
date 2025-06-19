%{
#include "inc/asembler.hpp"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

void yyerror(const char *s);
int yylex(void);
%}

%union {
    char* str;
    int num;
    struct jump_op {
        bool is_symbol;
        union {
            int literal;
            char* symbol;
        };
    } jop;
    struct data_op {
      int version;
      int gpr;
      union {
            int literal;
            char* symbol;
        };
    } dop;
}

%token <str> SYMBOL
%token <num> LITERAL
%token NEWLINE

%token DOT COMMA HASH
%token COLON DOLLAR PERCENT
%token OPEN_SQUARE_BRACKET CLOSE_SQUARE_BRACKET
%token PLUS

%token HALT INT IRET CALL RET JMP BEQ BNE BGT
%token PUSH POP XCHG ADD SUB MUL DIV
%token NOT AND OR XOR SHL SHR
%token LD ST CSRRD CSRWR

%token GLOBAL EXTERN SECTION WORD SKIP END

%token R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13 R14 SP R15 PC
%token STATUS HANDLER CAUSE

%type <jop> jump_operand
%type <dop> data_operand
%type <num> gpr
%type <num> csr

%start assembly_data

%%

assembly_data: | assembly_data assembly_line;

assembly_line: NEWLINE
             | directive NEWLINE
             | statement NEWLINE
             | label NEWLINE
;

directive: DOT directive_line;

statement: HALT {writeInstr(0x00, 0x00, 0x00, 0x00, 0x00, 0x0000);}
     | INT {writeInstr(0x01, 0x00, 0x00, 0x00, 0x00, 0x0000);}
     | IRET { 
      writeInstr(0x09, 0x03, 0x0F, 0x0E, 0x00, 0x0004);
      writeInstr(0x09, 0x07, 0x00, 0x0E, 0x00, 0x0004);
      }
     | CALL jump_operand {
      if ($2.is_symbol){
            writeFirstTwoBytesOfTheInstr(0x02, 0x00, 0x0F, 0x00);
            reportSymUsage($2.symbol, RelocationType::R_X86_64_PC32, 0x00, 2);
      } else {
            writeFirstTwoBytesOfTheInstr(0x02, 0x01, 0x0F, 0x00);
            reportLiteralUsage($2.literal, 0x00);
      }
      }
     | RET {writeInstr(0x09, 0x03, 0x0F, 0x0E, 0x00, 0x0004);}
     | JMP jump_operand {
      writeFirstTwoBytesOfTheInstr(0x03, 0x00, 0x0F, 0x00);
      if ($2.is_symbol){
            reportSymUsage($2.symbol, RelocationType::R_X86_64_PC32, 0x00, 2);
      } else {
            reportLiteralUsage($2.literal, 0x00);
      }
      }
     | BEQ gpr COMMA gpr COMMA jump_operand {
      uint32_t reg_b = static_cast<uint32_t>($2);
      uint32_t reg_c = static_cast<uint32_t>($4);
      writeFirstTwoBytesOfTheInstr(0x03, 0x01, 0x0F, $2);
      if ($6.is_symbol){
            reportSymUsage($6.symbol, RelocationType::R_X86_64_PC32, $4, 2);
      } else {
            reportLiteralUsage($6.literal, $4);
      }
      }
     | BNE gpr COMMA gpr COMMA jump_operand {
      uint32_t reg_b = static_cast<uint32_t>($2);
      uint32_t reg_c = static_cast<uint32_t>($4);
      writeFirstTwoBytesOfTheInstr(0x03, 0x02, 0x0F, $2);
      if ($6.is_symbol){
            reportSymUsage($6.symbol, RelocationType::R_X86_64_PC32, $4, 2);
      } else {
            reportLiteralUsage($6.literal, $4);
      }
      }
     | BGT gpr COMMA gpr COMMA jump_operand {
      uint32_t reg_b = static_cast<uint32_t>($2);
      uint32_t reg_c = static_cast<uint32_t>($4);
      writeFirstTwoBytesOfTheInstr(0x03, 0x03, 0x0F, $2);
      if ($6.is_symbol){
            reportSymUsage($6.symbol, RelocationType::R_X86_64_PC32, $4, 2);
      } else {
            reportLiteralUsage($6.literal, $4);
      }
      }
     | PUSH gpr {
      writeInstr(0x08, 0x01, 0x0E, 0x00, $2, 0xFFFC);
      }
     | POP gpr {
      writeInstr(0x09, 0x03, $2, 0x0E, 0x00, 0x0004);
      }
     | XCHG gpr COMMA gpr {
      writeInstr(0x04, 0x00, 0x00, $2, $4, 0x0000);
      }
     | ADD gpr COMMA gpr {
      writeInstr(0x05, 0x00, $4, $2, $4, 0x0000);
      }
     | SUB gpr COMMA gpr {
      writeInstr(0x05, 0x01, $4, $2, $4, 0x0000);
      }
     | MUL gpr COMMA gpr {
      writeInstr(0x05, 0x02, $4, $2, $4, 0x0000);
      }
     | DIV gpr COMMA gpr {
      writeInstr(0x05, 0x03, $4, $2, $4, 0x0000);
      }
     | NOT gpr {
      writeInstr(0x06, 0x00, $2, $2, 0x00, 0x0000);
      }
     | AND gpr COMMA gpr {
      writeInstr(0x06, 0x01, $4, $2, $4, 0x0000);
      }
     | OR gpr COMMA gpr {
      writeInstr(0x06, 0x02, $4, $2, $4, 0x0000);
      }
     | XOR gpr COMMA gpr {
      writeInstr(0x06, 0x03, $4, $2, $4, 0x0000);
      }
     | SHL gpr COMMA gpr {
      writeInstr(0x07, 0x00, $4, $4, $2, 0x000);
      }
     | SHR gpr COMMA gpr {
      writeInstr(0x07, 0x01, $4, $4, $2, 0x000);
      }
     | LD data_operand COMMA gpr {
      uint32_t lit_val = 0;
      switch($2.version){
            case 1:
                  writeFirstTwoBytesOfTheInstr(0x09, 0x02, $4, 0x0F);
                  reportLiteralUsage($2.literal, 0x00);
                  break;
            case 2:
                  writeFirstTwoBytesOfTheInstr(0x09, 0x01, $4, 0x0F);
                  reportSymUsage($2.symbol, RelocationType::R_X86_64_PC32, 0x00, 2);
                  break;
            case 3:
                  writeFirstTwoBytesOfTheInstr(0x09, 0x02, $4, 0x0F);
                  reportLiteralUsage($2.literal, 0x00);
                  writeInstr(0x09, 0x02, $4, $4, 0x00, 0x0000);
                  break;
            case 4:
                  writeFirstTwoBytesOfTheInstr(0x09, 0x01, $4, 0x0F);
                  reportSymUsage($2.symbol, RelocationType::R_X86_64_PC32, 0x00, 2);
                  writeInstr(0x09, 0x02, $4, $4, 0x00, 0x0000);
                  break;
            case 5:
                  writeInstr(0x09, 0x01, $4, $2.gpr, 0x00, 0x0000);
                  break;
            case 6:
                  writeInstr(0x09, 0x03, $4, $2.gpr, 0x00, 0x0000);
                  break;
            case 7:
                  lit_val = static_cast<uint32_t>($2.literal);
                  if ((lit_val <= 0x000007FF) || (lit_val >= 0xFFFFF800)) {
                        writeInstr(0x09, 0x02, $4, $2.gpr, 0x00, static_cast<uint16_t>(lit_val && 0x0FFF));
                  } else {
                        printf("ERROR - Literal cannot fit in 12 bits!\n");
                  }
                 break;
            case 8:
                  writeFirstTwoBytesOfTheInstr(0x09, 0x02, $4, $2.gpr);
                  reportSymUsage($2.symbol, RelocationType::R_X86_64_PC32, 0x0F, 2);
                  break;
            default:
                  printf("INVALID FORMAT OF LD INSTR!\n");
                   
      }
      }
     | ST gpr COMMA data_operand {
      uint32_t lit_val = 0;
      switch($4.version){
            case 3:
                 writeFirstTwoBytesOfTheInstr(0x08, 0x02, 0x0F, 0x00);
                 reportLiteralUsage($4.literal, $2);
                 break;
            case 4:
                  writeFirstTwoBytesOfTheInstr(0x08, 0x00, 0x0F, 0x00);
                  reportSymUsage($4.symbol, RelocationType::R_X86_64_PC32, $2, 2);
                  break;
            case 5:
                  writeInstr(0x09, 0x01, $4.gpr, $2, 0x00, 0x0000);
                  break;
            case 6:
                  writeInstr(0x08, 0x01, $4.gpr, 0x00, $2, 0x0000);
                  break;
            case 7:
                  lit_val = static_cast<uint32_t>($4.literal);
                  if ((lit_val <= 0x000007FF) || (lit_val >= 0xFFFFF800)) {
                        writeInstr(0x08, 0x00, $4.gpr, 0x00, $2, static_cast<uint16_t>(lit_val && 0x0FFF));
                  } else {
                        printf("ERROR - Literal cannot fit in 12 bits!\n");
                  }
                 break;
            case 8:
                  writeFirstTwoBytesOfTheInstr(0x08, 0x00, 0x0F, 0x00);
                  reportSymUsage($4.symbol, RelocationType::R_X86_64_PC32, $2, 2);
                  break;
            default:
                  printf("INVALID FORMAT OF ST INSTR!\n");
      }
      }
     | CSRRD csr COMMA gpr {
      writeInstr(0x09, 0x00, $4, $2, 0x00, 0x0000);
      }
     | CSRWR gpr COMMA csr {
      writeInstr(0x09, 0x04, $4, $2, 0x00, 0x0000);
      }
;

label: SYMBOL COLON { defineSym($1); }
     | SYMBOL COLON {defineSym($1);} statement
     | SYMBOL COLON {defineSym($1);} directive
;


directive_line: GLOBAL global_symbol_list {
      }
      | EXTERN extern_symbol_list {
      }
      | SECTION SYMBOL {
            std::string sym_name = $2;
            SymbolBinding bind = SymbolBinding::LOC;
            SymbolType type = SymbolType::SCTN;
            std::string sctn_name = sym_name;
            uint32_t value = 0;
            bool defined = true;
            closeCurrentSection();
            addSym(Sym(sym_name, bind, type, sctn_name, value, defined));
            openNewSection($2);
            }
      | WORD symbol_literal_list
      | SKIP LITERAL {
            for(uint8_t i = 0; i < $2; i++){
                  writeByte(0x00);
            }
            adjustLocation($2);
      }
      | END {
            closeCurrentSection();
            backPatch();
            printAll();
            return 0;
      }
;

gpr:
      R0   { $$ = 0; }
    | R1   { $$ = 1; }
    | R2   { $$ = 2; }
    | R3   { $$ = 3; }
    | R4   { $$ = 4; }
    | R5   { $$ = 5; }
    | R6   { $$ = 6; }
    | R7   { $$ = 7; }
    | R8   { $$ = 8; }
    | R9   { $$ = 9; }
    | R10  { $$ = 10; }
    | R11  { $$ = 11; }
    | R12  { $$ = 12; }
    | R13  { $$ = 13; }
    | SP   { $$ = 14; }
    | R14  { $$ = 14; }
    | PC   { $$ = 15; }
    | R15  { $$ = 15; }
;

csr: STATUS {$$ = 0;}
      | HANDLER {$$ = 1;}
      | CAUSE {$$ = 2;}
;

jump_operand: LITERAL {$$ = {false, .literal = $1};}
      | SYMBOL {$$ = {true, .symbol = $1};}
;

data_operand: DOLLAR LITERAL {
      $$ = {1, 0, .literal = $2};
      }
      | DOLLAR SYMBOL {
            $$ = {2, 0, .symbol = $2};
      }
      | LITERAL {
            $$ = {3, 0, .literal = $1};
      }
      | SYMBOL {
            $$ = {4, 0, .symbol = $1};
      }
      | gpr {
            $$ = {5, $1, .literal = -1};
      }
      | OPEN_SQUARE_BRACKET gpr CLOSE_SQUARE_BRACKET {$$ = {6, $2, .literal = 0};}
      | OPEN_SQUARE_BRACKET gpr PLUS LITERAL CLOSE_SQUARE_BRACKET {
            $$ = {7, $2, .literal = $4};
            }
      | OPEN_SQUARE_BRACKET gpr PLUS SYMBOL CLOSE_SQUARE_BRACKET {
             $$ = {8, $2, .symbol = $4};
            }
;

global_symbol_list: SYMBOL {
      std::string sym_name = $1;
      SymbolBinding bind = SymbolBinding::GLOB;
      addSym(Sym(sym_name, bind));
      reportGlobalSym(sym_name);
}
      | global_symbol_list COMMA SYMBOL {
            std::string sym_name = $3;
            SymbolBinding bind = SymbolBinding::GLOB;
            addSym(Sym(sym_name, bind));
            reportGlobalSym(sym_name);
      }
;

extern_symbol_list: SYMBOL {
      std::string sym_name = $1;
      SymbolBinding bind = SymbolBinding::GLOB;
      SymbolType type = SymbolType::NOTYPE;
      const std::string sctn_name = UNDEFINED_SCTN;
      addSym(Sym(sym_name, bind, type, sctn_name));
      reportExternSym(sym_name);
}
      | extern_symbol_list COMMA SYMBOL {
            std::string sym_name = $3;
            SymbolBinding bind = SymbolBinding::GLOB;
            SymbolType type= SymbolType::NOTYPE;
            const std::string sctn_name = UNDEFINED_SCTN;
            addSym(Sym(sym_name, bind, type, sctn_name));
            reportExternSym(sym_name);
      }
;

symbol_literal_list: sym_lit_list_leaf
      | symbol_literal_list COMMA sym_lit_list_leaf
;

sym_lit_list_leaf: SYMBOL {
      uint32_t offset = location_counter;
      std::string sym_name = $1;
      RelocationType rela_type = RelocationType::R_X86_64_32; 
      int32_t addend = 0;
      reportSymUsage(sym_name, rela_type, 0x00, addend);
      }
      | LITERAL {
            writeWord(static_cast<uint32_t>($1));
            adjustLocation(4);
            }
;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parser error: %s\n", s);
}

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file = "out.o";

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
        std::cerr << "Greška: neispravan broj argumenata.\n";
        std::cerr << "Korišćenje:\n";
        std::cerr << "  ./asembler ulaz.s\n";
        std::cerr << "  ./asembler -o izlaz.o ulaz.s\n";
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

    std::ofstream out(output_file);
    if (!out) {
        std::cerr << "Greška: ne mogu da otvorim izlaznu datoteku " << output_file << "\n";
        return 1;
    }

    out.close();
    fclose(yyin);

    return 0;
}
