#pragma once

enum OpCode{
  HALT,
  INT,
  CALL,
  JMP,
  XCHG,
  ARITHMETIC,
  LOGIC,
  SHIFT,
  ST,
  LD
};

enum CallMod{
  CALL_PC_REL,
  CALL_MEM_REL
};

enum JmpMod{
  JMP_PC_REL,
  BEQ_PC_REL,
  BNE_PC_REL,
  BGT_PC_REL,
  JMP_MEM_REL,
  BEQ_MEM_REL,
  BNE_MEM_REL,
  BGT_MEM_REL
};

enum ArithmeticMod{
  ADD,
  SUB,
  MUL,
  DIV
};

enum LogicMod{
  NOT,
  AND,
  OR,
  XOR
};

enum ShiftMod{
  SHL,
  SHR
};

enum StMod{
  MEM_REL,
  MEM_IND_DISP,
  MEM_IND
};

enum LdMod{
  GPR_DIR,
  GPR_PC_REL,
  GPR_MEM_IND,
  GPR_MEM_IND_DISP,
  CSR_DIR,
  CSR_PC_REL,
  CSR_MEM_IND,
  CSR_MEM_IND_DISP
};

enum Csr{
  STATUS,
  HANDLER,
  CAUSE
};

constexpr uint8_t ZERO = 0;
constexpr uint8_t SP = 0x0E;
constexpr uint8_t PC = 0x0F;
constexpr uint16_t WORD_SIZE = 0x0004;
constexpr uint16_t NEGATIVE_WORD_SIZE = 0xFFFC;
