%{
#include "inc/asembler.hpp"
#include <stdio.h>
#include <stdlib.h>

void yyerror(const char *s);
int yylex(void);
%}

%union {
    char* str;
    int num;
}

%token <str> SYMBOL
%token <num> LITERAL
%token NEWLINE

%token DOT COMMA HASH
%token COLON DOLLAR PERCENT
%token OPEN_SQUARE_BRACKET CLOSE_SQUARE_BRACKET
%token PLUS

// Instructions
%token HALT INT IRET CALL RET JMP BEQ BNE
%token PUSH POP XCHG ADD SUB MUL DIV
%token NOT AND OR XOR SHL SHR
%token LD ST CSRRD CSRWR

// Directives
%token GLOBAL EXTERN SECTION WORD SKIP END

// Registers
%token R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13 R14 SP R15 PC
%token STATUS HANDLER CAUSE


%start assembly_data

%%

assembly_data: {printf("EMPTY assembly_data\n");}| assembly_data assembly_line {printf("assembly_data assembly_line\n");}

assembly_line: NEWLINE { printf("Parsed: NEWLINE\n\n"); }
             | directive NEWLINE     { printf("Parsed: directive\n"); }
             | statement NEWLINE      { printf("Parsed: statement\n"); }
             | label NEWLINE      { printf("Parsed: label\n"); }
;

directive: DOT directive_line;

statement: HALT {printf("HALT Instruction\n");}
     | INT {printf("INT Instruction\n");}
     | IRET {printf("IRET Instruction\n");}
     | CALL jump_operand {printf("CALL Instruction\n");}
     | RET {printf("RET Instruction\n");}
     | JMP jump_operand {printf("JMP Instruction\n");}
     | BEQ gpr COMMA gpr COMMA jump_operand {printf("BEQ Instruction\n");}
     | BNE gpr COMMA gpr COMMA jump_operand {printf("BNE Instruction\n");}
     | PUSH gpr {printf("PUSH Instruction\n");}
     | POP gpr {printf("POP Instruction\n");}
     | XCHG gpr COMMA gpr {printf("XCHG Instruction\n");}
     | ADD gpr COMMA gpr {printf("ADD Instruction\n");}
     | SUB gpr COMMA gpr {printf("SUB Instruction\n");}
     | MUL gpr COMMA gpr {printf("MUL Instruction\n");}
     | DIV gpr COMMA gpr {printf("DIV Instruction\n");}
     | NOT gpr {printf("NOT Instruction\n");}
     | AND gpr COMMA gpr {printf("AND Instruction\n");}
     | OR gpr COMMA gpr {printf("OR Instruction\n");}
     | XOR gpr COMMA gpr {printf("XOR Instruction\n");}
     | SHL gpr COMMA gpr {printf("SHL Instruction\n");}
     | SHR gpr COMMA gpr {printf("SHR Instruction\n");}
     | LD data_operand COMMA gpr {printf("LD Instruction\n");}
     | ST gpr COMMA data_operand {printf("ST Instruction\n");}
     | CSRRD csr COMMA gpr {printf("CSRRD Instruction\n");}
     | CSRWR gpr COMMA csr {printf("CSRWR Instruction\n");}
;

label: SYMBOL COLON { printf("Label with symbol %s\n", $1); }
     | SYMBOL COLON statement { printf("Label with symbol %s\n", $1); }
     | SYMBOL COLON directive { printf("Label with symbol %s\n", $1); }
;

directive_line: GLOBAL { current_directive = "global"; } symbol_list {
            printf("GLOBAL directive\n");
      }
      | EXTERN { current_directive = "extern"; } symbol_list {
            printf("EXTERN directive\n");
      }
      | SECTION section_name {printf("SECTION directive\n");}
      | WORD symbol_literal_list {printf("WORD directive\n");}
      | SKIP LITERAL {printf("SKIP directive\n");}
      | END {printf("END directive\n");
             printSymbolLists();
            }
;

gpr: R0 
      | R1 {printf("Found R1\n");}
      | R2 {printf("Found R2\n");}
      | R3 {printf("Found R3\n");}
      | R4 {printf("Found R4\n");}
      | R5 {printf("Found R5\n");}
      | R6 {printf("Found R6\n");}
      | R7 {printf("Found R7\n");}
      | R8 {printf("Found R8\n");}
      | R9 {printf("Found R9\n");}
      | R10 {printf("Found R10\n");}
      | R11 {printf("Found R11\n");}
      | R12 {printf("Found R12\n");}
      | R13 {printf("Found R13\n");}
      | R14 {printf("Found R14\n");}
      | SP {printf("Found SP\n");}
      | R15 {printf("Found R15\n");}
      | PC {printf("Found PC\n");}
;

csr: STATUS {printf("Found STATUS reg\n");}
      | HANDLER {printf("Found HANDLER reg\n");}
      | CAUSE {printf("Found CAUSE reg\n");}
;

jump_operand: LITERAL {printf("Found literal %d\n", $1);}
      | SYMBOL {printf("Found symbol %s\n", $1);}
;

data_operand: DOLLAR LITERAL {printf("Found literal with dollar $ - $%d\n", $2);}
      | DOLLAR SYMBOL {printf("Found symbol with dollar $ - $%s\n", $2);}
      | LITERAL {printf("Found literal 0x%X\n", $1);}
      | SYMBOL {printf("Found symbol %s\n", $1);}
      | reg
      | OPEN_SQUARE_BRACKET reg CLOSE_SQUARE_BRACKET
      | OPEN_SQUARE_BRACKET reg PLUS LITERAL CLOSE_SQUARE_BRACKET {printf("reg + literal %d inside []", $4);}
      | OPEN_SQUARE_BRACKET reg PLUS SYMBOL CLOSE_SQUARE_BRACKET {printf("reg + symbol %s inside []", $4);}
;

symbol_list: SYMBOL {
      printf("Found symbol %s\n", $1);
      insertIntoCurrentDirectiveList($1);
}
      | symbol_list COMMA SYMBOL {
            printf("Found symbol %s\n", $3);
            insertIntoCurrentDirectiveList($3);
      }
;

symbol_literal_list: sym_lit_list_leaf
      | symbol_literal_list COMMA sym_lit_list_leaf
;

sym_lit_list_leaf: SYMBOL {printf("Found symbol %s\n", $1);}
      | LITERAL {printf("Found literal %d\n", $1);}
;

section_name: SYMBOL {printf("Found section with a symbol name %s\n", $1);};

reg: gpr
      | csr
;


%%

void yyerror(const char *s) {
    fprintf(stderr, "Parser error: %s\n", s);
}

int main() {
    return yyparse();
}
