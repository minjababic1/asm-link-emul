#include "../inc/asembler.hpp"
#include "../inc/asembler_instr.hpp"
#include "../inc/instructions.hpp"

#include <cstdint>
#include <iostream>

void halt_(){
  writeInstruction(OpCode::HALT, ZERO, ZERO, ZERO, ZERO, ZERO);
}

void int_(){
  writeInstruction(OpCode::INT, ZERO, ZERO, ZERO, ZERO, ZERO);
}

void iret_(){
  writeInstruction(OpCode::LD, LdMod::CSR_MEM_IND, Csr::STATUS, SP, ZERO, WORD_SIZE);
  writeInstruction(OpCode::LD, LdMod::GPR_MEM_IND_DISP, PC, SP, ZERO, 2 * WORD_SIZE);
}

void call_(){
  writeInstructionFixedFields(OpCode::CALL, CallMod::CALL_MEM_REL, PC, ZERO, ZERO);
}

void ret_(){
  writeInstruction(OpCode::LD, LdMod::GPR_MEM_IND_DISP, PC, SP, ZERO, WORD_SIZE);
}

void jmp_(){
  writeInstructionFixedFields(OpCode::JMP, JmpMod::JMP_MEM_REL, PC, ZERO, ZERO);
}

void beq_(uint8_t a_gpr_1, uint8_t a_gpr_2){
  writeInstructionFixedFields(OpCode::JMP, JmpMod::BEQ_MEM_REL, PC, a_gpr_1, a_gpr_2);
}

void bne_(uint8_t a_gpr_1, uint8_t a_gpr_2){
  writeInstructionFixedFields(OpCode::JMP, JmpMod::BNE_MEM_REL, PC, a_gpr_1, a_gpr_2);
}

void bgt_(uint8_t a_gpr_1, uint8_t a_gpr_2){
  writeInstructionFixedFields(OpCode::JMP, JmpMod::BGT_MEM_REL, PC, a_gpr_1, a_gpr_2);
}

void push_(uint8_t a_gpr){
  writeInstruction(OpCode::ST, StMod::MEM_IND_DISP, SP, ZERO, a_gpr, NEGATIVE_WORD_SIZE);
}

void pop_(uint8_t a_gpr){
  writeInstruction(OpCode::LD, LdMod::GPR_MEM_IND_DISP, a_gpr, SP, ZERO, WORD_SIZE);
}

void xchg_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::XCHG, ZERO, ZERO, a_gpr_s, a_gpr_d, ZERO);
}

void add_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::ARITHMETIC, ArithmeticMod::ADD, a_gpr_d, a_gpr_d, a_gpr_s, ZERO);
}

void sub_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::ARITHMETIC, ArithmeticMod::SUB, a_gpr_d, a_gpr_d, a_gpr_s, ZERO);
}

void mul_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::ARITHMETIC, ArithmeticMod::MUL, a_gpr_d, a_gpr_d, a_gpr_s, ZERO);
}

void div_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::ARITHMETIC, ArithmeticMod::DIV, a_gpr_d, a_gpr_d, a_gpr_s, ZERO);
}

void not_(uint8_t a_gpr){
  writeInstruction(OpCode::LOGIC, LogicMod::NOT, a_gpr, a_gpr, ZERO, ZERO);
}

void and_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::LOGIC, LogicMod::AND, a_gpr_d, a_gpr_d, a_gpr_s, ZERO);
}

void or_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::LOGIC, LogicMod::OR, a_gpr_d, a_gpr_d, a_gpr_s, ZERO);
}

void xor_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::LOGIC, LogicMod::XOR, a_gpr_d, a_gpr_d, a_gpr_s, ZERO);
}

void shl_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::SHIFT, ShiftMod::SHL, a_gpr_d, a_gpr_d, a_gpr_s, ZERO);
}

void shr_(uint8_t a_gpr_s, uint8_t a_gpr_d){
  writeInstruction(OpCode::SHIFT, ShiftMod::SHR, a_gpr_d, a_gpr_d, a_gpr_s, ZERO);
}

void ld_(
  uint8_t a_version, 
  uint32_t a_literal, 
  const std::string& a_sym_name, 
  uint8_t a_gpr,
  uint8_t a_gpr_d
){
  uint32_t lit_val = 0;
  uint32_t sym_val = 0;
  switch(a_version){
        case 1:
              writeInstructionFixedFields(OpCode::LD, LdMod::GPR_MEM_IND, a_gpr_d, PC, ZERO);
              handleInstructionLiteral(a_literal);
              break;
        case 2:
              writeInstructionFixedFields(OpCode::LD, LdMod::GPR_MEM_IND, a_gpr_d, PC, ZERO);
              handleInstructionSymbol(a_sym_name);
              break;
        case 3:
              writeInstructionFixedFields(OpCode::LD, LdMod::GPR_MEM_IND, a_gpr_d, PC, ZERO);
              handleInstructionLiteral(a_literal);
              writeInstruction(OpCode::LD, LdMod::GPR_MEM_IND, a_gpr_d, a_gpr_d, ZERO, ZERO);
              break;
        case 4:
              writeInstructionFixedFields(OpCode::LD, LdMod::GPR_MEM_IND, a_gpr_d, PC, ZERO);
              handleInstructionSymbol(a_sym_name);
              writeInstruction(OpCode::LD, LdMod::GPR_MEM_IND, a_gpr_d, a_gpr_d, ZERO, ZERO);
              break;
        case 5:
              writeInstruction(OpCode::LD, LdMod::GPR_PC_REL, a_gpr_d, a_gpr, ZERO, ZERO);
              break;
        case 6:
              writeInstruction(OpCode::LD, LdMod::GPR_MEM_IND_DISP, a_gpr_d, a_gpr, ZERO, ZERO);
              break;
        case 7:
              lit_val = static_cast<uint32_t>(a_literal);
              if ((lit_val <= 0x000007FF) || (lit_val >= 0xFFFFF800)) {
                    writeInstruction(OpCode::LD, LdMod::GPR_MEM_IND, a_gpr_d, a_gpr, ZERO, static_cast<uint16_t>(lit_val & 0x0FFF));
              } else {
                    printf("ERROR - Literal cannot fit in 12 bits!\n");
              }
              break;
        case 8:
              sym_val = getSymbolValue(a_sym_name);
              if (isSymbolDefined(a_sym_name) && getSymbolSection(a_sym_name) == "#EQU") {
                if (sym_val <= 0xFFF) {
                  writeInstruction(OpCode::LD, LdMod::GPR_MEM_IND, a_gpr_d, a_gpr, ZERO, static_cast<uint16_t>(sym_val & 0x0FFF));
                } else {
                  printf("ERROR - Symbol value cannot fit in 12 bits!\n");
                }
              } else {
                printf("ERROR - Symbol value is not known in the compile time!\n");
              }
        default:
              printf("INVALID FORMAT OF LD INSTR!\n"); 
  }
}

void st_(
  uint8_t a_version,
  uint32_t a_literal,
  const std::string& a_sym_name,
  uint8_t a_gpr,
  uint8_t a_gpr_s
){
  uint32_t lit_val = 0;
  uint32_t sym_val = 0;
  switch(a_version){
        case 3:
              writeInstructionFixedFields(OpCode::ST, StMod::MEM_IND, PC, ZERO, a_gpr_s);
              handleInstructionLiteral(a_literal);
              break;
        case 4:
              writeInstructionFixedFields(OpCode::ST, StMod::MEM_IND, PC, ZERO, a_gpr_s);
              handleInstructionSymbol(a_sym_name);
              break;
        case 5:
              writeInstruction(OpCode::LD, LdMod::GPR_PC_REL, a_gpr, a_gpr_s, ZERO, ZERO);
              break;
        case 6:
              writeInstruction(OpCode::ST, StMod::MEM_IND_DISP, a_gpr, ZERO, a_gpr_s, ZERO);
              break;
        case 7:
              lit_val = static_cast<uint32_t>(a_literal);
              if ((lit_val <= 0x000007FF) || (lit_val >= 0xFFFFF800)) {
                    writeInstruction(OpCode::ST, StMod::MEM_REL, a_gpr, ZERO, a_gpr_s, static_cast<uint16_t>(lit_val && 0x0FFF));
              } else {
                    printf("ERROR - Literal cannot fit in 12 bits!\n");
              }
              break;
        case 8:
              sym_val = getSymbolValue(a_sym_name);
              if (isSymbolDefined(a_sym_name) && getSymbolSection(a_sym_name) == "#EQU") {
                if (sym_val <= 0xFFF) {
                  writeInstruction(OpCode::ST, StMod::MEM_REL, a_gpr, ZERO, a_gpr_s, static_cast<uint16_t>(sym_val && 0x0FFF));
                } else {
                  printf("ERROR - Symbol value cannot fit in 12 bits!\n");
                }
              } else {
                printf("ERROR - Symbol value is not known in the compile time!\n");
              }
        default:
              printf("INVALID FORMAT OF ST INSTR!\n");
  }
}

void csrrd_(uint8_t a_gpr, uint8_t a_csr){
  writeInstruction(OpCode::LD, LdMod::GPR_DIR, a_gpr, a_csr, ZERO, ZERO);
}

void csrwr_(uint8_t a_gpr, uint8_t a_csr){
  writeInstruction(OpCode::LD, LdMod::CSR_DIR, a_csr, a_gpr, ZERO, ZERO);
}
