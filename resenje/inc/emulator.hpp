#pragma once

#include <climits>
#include <stdlib.h>
#include <stdint.h>
#include <map>

constexpr std::size_t GPR_NUM = 16;
constexpr std::size_t CSR_NUM = 3;

struct Instruction{
  uint8_t m_oc;
  uint8_t m_mod;
  uint8_t m_reg_a;
  uint8_t m_reg_b;
  uint8_t m_reg_c;
  int16_t m_disp;

  Instruction() : 
    m_oc(0), m_mod(0), m_reg_a(0), 
    m_reg_b(0), m_reg_c(0), m_disp(0) {} 

  Instruction(uint8_t a_oc, uint8_t a_mod, uint8_t a_reg_a,
                uint8_t a_reg_b, uint8_t a_reg_c, uint16_t a_disp)
        : m_oc(a_oc), m_mod(a_mod), m_reg_a(a_reg_a),
          m_reg_b(a_reg_b), m_reg_c(a_reg_c), m_disp(a_disp) {}
};

struct Emulator{
  std::map<uint32_t, uint8_t> m_mem32;
  uint32_t m_gpr[GPR_NUM];
  uint32_t m_csr[CSR_NUM];
};
