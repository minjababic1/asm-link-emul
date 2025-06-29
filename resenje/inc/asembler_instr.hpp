#pragma once
#include "asembler.hpp"

void halt_();
void int_();
void iret_();
void call_();
void ret_();
void jmp_();
void beq_(uint8_t a_gpr_1, uint8_t a_gpr_2);
void bne_(uint8_t a_gpr_1, uint8_t a_gpr_2);
void bgt_(uint8_t a_gpr_1, uint8_t a_gpr_2);
void push_(uint8_t a_gpr);
void pop_(uint8_t a_gpr);
void xchg_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void add_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void sub_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void mul_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void div_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void not_(uint8_t a_gpr);
void and_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void or_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void xor_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void shl_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void shr_(uint8_t a_gpr_s, uint8_t a_gpr_d);
void ld_(
  uint8_t a_version, 
  uint32_t a_literal, 
  const std::string& a_sym_name, 
  uint8_t a_gpr,
  uint8_t a_gpr_d
);
void st_(
  uint8_t a_version,
  uint32_t a_literal,
  const std::string& a_sym_name,
  uint8_t a_gpr,
  uint8_t a_gpr_s
);
void csrrd_(uint8_t a_gpr, uint8_t a_csr);
void csrwr_(uint8_t a_gpr, uint8_t a_csr);
