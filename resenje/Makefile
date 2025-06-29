BUILD_DIR := build
SRC_DIR := src
TEST_DIR := tests
NIVO_A := nivo-a
FLEX_SRC := misc/flex/asm.l
BISON_SRC := misc/bison/asm.y
ASM := asembler
LINK := linker
EMU := emulator


all: clean $(BUILD_DIR)/$(LINK)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/asm.tab.c $(BUILD_DIR)/asm.tab.h: $(BISON_SRC) | $(BUILD_DIR)
	bison -t -v -d -o $(BUILD_DIR)/asm.tab.c --defines=$(BUILD_DIR)/asm.tab.h $(BISON_SRC)

$(BUILD_DIR)/lex.yy.c: $(FLEX_SRC) $(BUILD_DIR)/asm.tab.h | $(BUILD_DIR)
	flex -o $(BUILD_DIR)/lex.yy.c $(FLEX_SRC)

$(BUILD_DIR)/$(ASM): $(BUILD_DIR)/lex.yy.c $(BUILD_DIR)/asm.tab.c $(BUILD_DIR)/asm.tab.h
	g++ -std=c++17 -o $(BUILD_DIR)/$(ASM) \
		$(BUILD_DIR)/asm.tab.c $(BUILD_DIR)/lex.yy.c \
		$(SRC_DIR)/asembler.cpp $(SRC_DIR)/asembler_instr.cpp \
		$(SRC_DIR)/asembler_dir.cpp $(SRC_DIR)/common.cpp

$(BUILD_DIR)/$(LINK): $(BUILD_DIR)/$(ASM)
	g++ -std=c++17 -o $(BUILD_DIR)/$(LINK) $(SRC_DIR)/linker.cpp $(SRC_DIR)/common.cpp

nivo-a: all
	./$(BUILD_DIR)/$(ASM) -o $(BUILD_DIR)/main.o $(TEST_DIR)/$(NIVO_A)/main.s
	./$(BUILD_DIR)/$(ASM) -o $(BUILD_DIR)/math.o $(TEST_DIR)/$(NIVO_A)/math.s
	./$(BUILD_DIR)/$(ASM) -o $(BUILD_DIR)/handler.o $(TEST_DIR)/$(NIVO_A)/handler.s
	./$(BUILD_DIR)/$(ASM) -o $(BUILD_DIR)/isr_timer.o $(TEST_DIR)/$(NIVO_A)/isr_timer.s
	./$(BUILD_DIR)/$(ASM) -o $(BUILD_DIR)/isr_terminal.o $(TEST_DIR)/$(NIVO_A)/isr_terminal.s
	./$(BUILD_DIR)/$(ASM) -o $(BUILD_DIR)/isr_software.o $(TEST_DIR)/$(NIVO_A)/isr_software.s
	./$(BUILD_DIR)/$(LINK) -hex -o $(BUILD_DIR)/program.hex \
		-place=my_code@0x40000000 -place=math@0xF0000000 \
		$(BUILD_DIR)/handler.o $(BUILD_DIR)/math.o $(BUILD_DIR)/main.o \
		$(BUILD_DIR)/isr_terminal.o $(BUILD_DIR)/isr_timer.o $(BUILD_DIR)/isr_software.o

clean:
	rm -rf $(BUILD_DIR)
