%{
#include "asm.tab.h"
#include <string.h>
%}

digit       [0-9]
alpha       [a-zA-Z_]
alphanum    [a-zA-Z0-9_]
symbol      {alpha}{alphanum}*

%%

"halt"      { return HALT; }
"int"       { return INT; }
"iret"      { return IRET; }
"call"      { return CALL; }
"ret"       { return RET; }
"jmp"       { return JMP; }
"beq"       { return BEQ; }
"bne"       { return BNE; }
"bgt"       { return BGT; }
"push"      { return PUSH; }
"pop"       { return POP; }
"xchg"      { return XCHG; }
"add"       { return ADD; }
"sub"       { return SUB; }
"mul"       { return MUL; }
"div"       { return DIV; }
"not"       { return NOT; }
"and"       { return AND; }
"or"        { return OR; }
"xor"       { return XOR; }
"shl"       { return SHL; }
"shr"       { return SHR; }
"ld"        { return LD; }
"st"        { return ST; }
"csrrd"     { return CSRRD; }
"csrwr"     { return CSRWR; }

".global"   { return GLOBAL; }
".extern"   { return EXTERN; }
".section"  { return SECTION; }
".word"     { return WORD; }
".skip"     { return SKIP; }
".ascii"    { return ASCII; }
".equ"      { return EQU; }
".end"      { return END; }
"\""({symbol}|[ ])+"\""   { yylval.str = strdup(yytext); return TEXT;}

"%r0"       { return R0; }
"%r1"       { return R1; }
"%r2"       { return R2; }
"%r3"       { return R3; }
"%r4"       { return R4; }
"%r5"       { return R5; }
"%r6"       { return R6; }
"%r7"       { return R7; }
"%r8"       { return R8; }
"%r9"       { return R9; }
"%r10"      { return R10; }
"%r11"      { return R11; }
"%r12"      { return R12; }
"%r13"      { return R13; }
"%r14"      { return R14; }
"%sp"      { return SP; }
"%r15"      { return R15; }
"%pc"      { return PC; }

"%status"   { return STATUS; }
"%handler"  { return HANDLER; }
"%cause"    { return CAUSE; }

","         { return COMMA; }
":"         { return COLON; }
"$"         { return DOLLAR; }
"["         { return OPEN_SQUARE_BRACKET; }
"]"         { return CLOSE_SQUARE_BRACKET; }
"+"         { return PLUS; }
"-"         { return MINUS; }

0[xX][0-9a-fA-F]+ {
    yylval.num = (int)strtol(yytext, NULL, 16);  // baza 16 za hex
    return LITERAL;
}
{digit}+    { yylval.num = atoi(yytext); return LITERAL; }
{symbol}    { yylval.str = strdup(yytext); return SYMBOL; }

"#".*   ;

\n                  { return NEWLINE;}
[ \t]+  

.           { printf("Unknown char: %s\n", yytext); }

%%

int yywrap() {
    return 1;
}

