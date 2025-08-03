%{
#include "../inc/asembler.hpp"
#include "../inc/asembler_dir.hpp"
#include "../inc/asembler_instr.hpp"
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
    struct data_op {
      int version;
      int gpr;
      int literal;
      char* symbol;
    } dop;
}

%token <str> SYMBOL
%token <str> TEXT
%token <num> LITERAL
%token NEWLINE

%token COMMA HASH
%token COLON DOLLAR PERCENT
%token OPEN_SQUARE_BRACKET CLOSE_SQUARE_BRACKET
%token PLUS DOUBLE_QUOTE

%token HALT INT IRET CALL RET JMP BEQ BNE BGT
%token PUSH POP XCHG ADD SUB MUL DIV
%token NOT AND OR XOR SHL SHR
%token LD ST CSRRD CSRWR

%token GLOBAL EXTERN SECTION WORD SKIP ASCII END

%token R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13 R14 SP R15 PC
%token STATUS HANDLER CAUSE

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

statement: HALT { halt_(); }
     | INT { int_(); }
     | IRET { iret_(); }
     | CALL { call_();} jump_operand 
     | RET { ret_(); }
     | JMP { jmp_(); } jump_operand
     | BEQ gpr COMMA gpr COMMA {
            uint8_t gpr_1 = static_cast<uint8_t>($2);
            uint8_t gpr_2 = static_cast<uint8_t>($4);
            beq_(gpr_1, gpr_2);
      } jump_operand
     | BNE gpr COMMA gpr COMMA {
            uint8_t gpr_1 = static_cast<uint8_t>($2);
            uint8_t gpr_2 = static_cast<uint8_t>($4);
            bne_(gpr_1, gpr_2);
      } jump_operand
     | BGT gpr COMMA gpr COMMA {
            uint8_t gpr_1 = static_cast<uint8_t>($2);
            uint8_t gpr_2 = static_cast<uint8_t>($4);
            bgt_(gpr_1, gpr_2);
      } jump_operand
     | PUSH gpr { 
            uint8_t gpr = $2;
            push_(gpr); 
      }
     | POP gpr {
            uint8_t gpr = $2;
            pop_(gpr);
      }
     | XCHG gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            xchg_(gpr_s, gpr_d);
      }
     | ADD gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            add_(gpr_s, gpr_d);
      }
     | SUB gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            sub_(gpr_s, gpr_d);
      }
     | MUL gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            mul_(gpr_s, gpr_d);
      }
     | DIV gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            div_(gpr_s, gpr_d);
      }
     | NOT gpr {
            uint8_t gpr = $2;
            not_(gpr); 
      }
     | AND gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            and_(gpr_s, gpr_d);
      }
     | OR gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            or_(gpr_s, gpr_d);      }
     | XOR gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            xor_(gpr_s, gpr_d);
      }
     | SHL gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            shl_(gpr_s, gpr_d);
      }
     | SHR gpr COMMA gpr {
            uint8_t gpr_s = static_cast<uint8_t>($2);
            uint8_t gpr_d = static_cast<uint8_t>($4);
            shr_(gpr_s, gpr_d);
      }
     | LD data_operand COMMA gpr {
            uint8_t version = $2.version;
            uint32_t literal = $2.literal;
            const std::string sym_name = ($2.symbol != nullptr) ? std::string($2.symbol) : "";
            uint8_t gpr = $2.gpr;
            uint8_t gpr_d = $4;
            ld_(version, literal, sym_name, gpr, gpr_d);
      }
     | ST gpr COMMA data_operand {
            uint8_t version = $4.version;
            uint32_t literal = $4.literal;
            const std::string sym_name = ($4.symbol != nullptr) ? std::string($4.symbol) : "";
            uint8_t gpr = $4.gpr;
            uint8_t gpr_s = $2;
            st_(version, literal, sym_name, gpr, gpr_s);
      }
     | CSRRD csr COMMA gpr {
            uint8_t gpr = $4;
            uint8_t csr = $2;
            csrrd_(gpr, csr);
      }
     | CSRWR gpr COMMA csr {
            uint8_t gpr = $2;
            uint8_t csr = $4;
            csrwr_(gpr, csr);
      }
;

label: SYMBOL COLON { defineSymbol($1, SymbolType::NOTYP); }
     | SYMBOL COLON { defineSymbol($1, SymbolType::NOTYP); } statement
     | SYMBOL COLON { defineSymbol($1, SymbolType::NOTYP); } directive
;


directive: GLOBAL global_symbol_list {
      }
      | EXTERN extern_symbol_list {
      }
      | SECTION SYMBOL {
            std::string sym_name = $2;
            section_(sym_name);
      }
      | WORD word_list
      | SKIP LITERAL { 
            uint32_t literal = $2;
            skip_(literal);
      }
      | ASCII TEXT {
            std::string text = $2;
            uint8_t text_size = text.size();
            std::string without_quotes = (text.substr(0, text_size-1)).substr(1);
            ascii_(without_quotes);
      }
      | END {
            end_();
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

jump_operand: LITERAL {handleInstructionLiteral($1);}
      | SYMBOL {handleInstructionSymbol($1);}
;

data_operand: DOLLAR LITERAL {
      $$ = {1, 0, $2, NULL};
      }
      | DOLLAR SYMBOL {
            $$ = {2, 0, 0, $2};
      }
      | LITERAL {
            $$ = {3, 0, $1, NULL};
      }
      | SYMBOL {
            $$ = {4, 0, 0, $1};
      }
      | gpr {
            $$ = {5, $1, 0, NULL};
      }
      | OPEN_SQUARE_BRACKET gpr CLOSE_SQUARE_BRACKET {$$ = {6, $2, 0, NULL};}
      | OPEN_SQUARE_BRACKET gpr PLUS LITERAL CLOSE_SQUARE_BRACKET {
            $$ = {7, $2, $4, NULL};
            }
;

global_symbol_list: SYMBOL {
      std::string sym_name = $1;
      global_(sym_name);
}
      | global_symbol_list COMMA SYMBOL {
            std::string sym_name = $3;
            global_(sym_name);
      }
;

extern_symbol_list: SYMBOL {
      std::string sym_name = $1;
      extern_(sym_name);
}
      | extern_symbol_list COMMA SYMBOL {
            std::string sym_name = $3;
            extern_(sym_name);
      }
;

word_list: word_list_leaf
      | word_list COMMA word_list_leaf
;

word_list_leaf: SYMBOL {
            std::string sym_name = $1;
            word_(sym_name);
      }
      | LITERAL { 
            uint32_t literal = $1;
            word_(literal);
      }
;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parser error: %s\n", s);
}
