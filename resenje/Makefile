all: asembler

asm.tab.c asm.tab.h: misc/bison/asm.y
	bison -t -v -d misc/bison/asm.y

lex.yy.c: misc/flex/asm.l asm.tab.h
	flex misc/flex/asm.l

asembler: lex.yy.c asm.tab.c asm.tab.h
	g++ -o asembler asm.tab.c lex.yy.c src/asembler.cpp
clean:
	rm asembler asm.tab.c lex.yy.c asm.tab.h asm.output
	rm -rf output
	rm out.o

