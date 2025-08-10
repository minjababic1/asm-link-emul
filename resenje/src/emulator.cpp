#include "../inc/emulator.hpp"
#include "../inc/emu_terminal.hpp"
#include "../inc/instructions.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

Emulator emulator;
const uint32_t term_out = 0xFFFFFF00;
const uint32_t term_in = 0xFFFFFF04;
const uint32_t tim_cfg = 0xFFFFFF10;

const uint32_t TIMER_MASK = 0x00000001;
const uint32_t TERMINAL_MASK = 0x00000002;
const uint32_t INTERRUPT_MASK = 0x00000003;

std::unordered_map<uint32_t, uint32_t> timer_config_map = {
  {0x0, 500},
  {0x1, 1000},
  {0x2, 1500},
  {0x3, 2000},
  {0x4, 5000},
  {0x5, 10000},
  {0x6, 30000},
  {0x7, 60000}
};

int32_t handleArguments(int argc, char* argv[], std::string& a_input_file) {
  if (argc != 2) {
    std::cerr << "Greska: Nedozvoljen broj argumenata" << std::endl;
    return 1;
  }
  a_input_file = std::string(argv[1]);
  return 0;
}

void writeMem32(uint32_t a_addr, uint8_t a_byte) {
  emulator.m_mem32[a_addr] = a_byte;
}

void writeWord(uint32_t a_addr, uint32_t a_word) {
  writeMem32(a_addr, static_cast<uint8_t>(a_word & 0xFF));
  writeMem32(a_addr + 1, static_cast<uint8_t>((a_word >> 8) & 0xFF));
  writeMem32(a_addr + 2, static_cast<uint8_t>((a_word >> 16) & 0xFF));
  writeMem32(a_addr + 3, static_cast<uint8_t>((a_word >> 24) & 0xFF));
}

uint8_t readMem32(uint32_t a_addr) {
  if (emulator.m_mem32.find(a_addr) != emulator.m_mem32.end()) {
    return emulator.m_mem32[a_addr];
  } else {
    return 0x00;
  }
}

uint32_t readWord(uint32_t a_addr) {
  return static_cast<uint32_t>(readMem32(a_addr)) | 
    (static_cast<uint32_t>(readMem32(a_addr + 1)) << 8) |
    (static_cast<uint32_t>(readMem32(a_addr + 2)) << 16) |
    (static_cast<uint32_t>(readMem32(a_addr + 3)) << 24);
}

uint32_t readInstr(uint32_t a_addr) {
  return (static_cast<uint32_t>(readMem32(a_addr)) << 24) | 
    (static_cast<uint32_t>(readMem32(a_addr + 1)) << 16) |
    (static_cast<uint32_t>(readMem32(a_addr + 2)) << 8) |
    static_cast<uint32_t>(readMem32(a_addr + 3));
}

void showMem32() {
  std::cout << std::uppercase << std::right << std::hex << std::setfill('0');
  for (const auto& [addr, byte] : emulator.m_mem32) {
    if (addr % 8 == 0) {
      std::cout << std::setw(8)  << addr << ": ";
    }

    std::cout << std::setw(2) << static_cast<uint16_t>(byte & 0x00FF);

    if (addr % 8 == 7) {
      std::cout << "\n";
    } else if (addr % 8 == 3) {
      std::cout << "   ";
    } else {
      std::cout << " ";
    }
  }

  std::cout << std::left << std::dec << std::setfill(' ');
}

void showEmulatorState() {
  std::cout << "-----------------------------------------------------------------" << std::endl;
  std::cout << "Emulated processor executed halt instruction" << std::endl;
  std::cout << "Emulated processor state:" << std::endl;
  for (size_t i = 0 ; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      std::string gpr_out = "r" + std::to_string(i*4+j) + "=0x";
      std::cout << std::uppercase << std::right << std::setw(6) << std::setfill(' ') << gpr_out;
      std::cout << std::setw(8) << std::hex << std::setfill('0') << emulator.m_gpr[i*4+j]
        << std::dec << std::setfill(' ');
      std::cout << "   ";
    }
    std::cout << std::endl;
  }
}

int32_t parseHexFile(std::string& a_input_file) {
  std::ifstream in(a_input_file);

  if (!in.is_open()) {
      std::cerr << "Greska prilikom otvaranja fajla: " << a_input_file << "\n";
      return 1;
  }

  std::string line;
  while(getline(in, line)) {
    std::istringstream iss(line);
    std::string addr_representation;
    std::string byte_representation;
    iss >> addr_representation;
    uint32_t addr_val = static_cast<uint32_t>(
      std::stoul(addr_representation.substr(0, addr_representation.size() - 1), nullptr, 16)
    );
    while (iss >> byte_representation) {
        uint8_t byte_val = static_cast<uint8_t>(std::stoul(byte_representation, nullptr, 16));
        writeMem32(addr_val, byte_val);
        addr_val++;
    }
  }

  in.close();
  return 0;
}

Instruction loadInstr(uint32_t& a_pc) {
  uint32_t word = readInstr(a_pc);
  a_pc+= WORD_SIZE;
  
  uint8_t oc = static_cast<uint8_t>((word & 0xF0000000) >> 28);
  uint8_t mod = static_cast<uint8_t>((word & 0x0F000000) >> 24);
  uint8_t reg_a = static_cast<uint8_t>((word & 0x00F00000) >> 20);
  uint8_t reg_b = static_cast<uint8_t>((word & 0x000F0000) >> 16);
  uint8_t reg_c = static_cast<uint8_t>((word & 0x0000F000) >> 12);
  int16_t disp = static_cast<int16_t>(static_cast<int32_t>(word << 20) >> 20);

  return Instruction(oc, mod, reg_a, reg_b, reg_c, disp);
}

void push(uint32_t a_val) {
  emulator.m_gpr[SP]-= 4;
  writeWord(emulator.m_gpr[SP], a_val);
}

void pop(uint32_t& a_dst) {
  a_dst = readWord(emulator.m_gpr[SP]);
  emulator.m_gpr[SP]+= 4;
}

void printInstruction(uint8_t a_oc) {
  switch (a_oc) {
    case OpCode::HALT:
      std::cout << "HALT instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    case OpCode::INT:
      std::cout << "INT instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    case OpCode::CALL:
      std::cout << "CALL instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    case OpCode::JMP:
      std::cout << "JMP instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    case OpCode::XCHG:
      std::cout << "XCHG instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    case OpCode::ARITHMETIC:
      std::cout << "ARITHMETIC instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    case OpCode::LOGIC:
      std::cout << "LOGIC instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    case OpCode::SHIFT:
      std::cout << "SHIFT instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    case OpCode::ST:
      std::cout << "ST instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    case OpCode::LD:
      std::cout << "LD instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
    default:
      std::cout << "Unknown instruction on " << std::hex << emulator.m_gpr[PC] << std::dec << std::endl;
      break;
  }
}

void execute() {
  emulator.m_gpr[PC] = 0x40000000;
  Instruction curr_instr;
  auto start_time = std::chrono::high_resolution_clock::now();
  auto end_time = start_time;
  int32_t timer_config = -1;
  bool timer_triggered = false;
  do {
    curr_instr = loadInstr(emulator.m_gpr[PC]);
    switch (curr_instr.m_oc) 
    {
      case OpCode::HALT:
        break;
      case OpCode::INT:
        // push status; push pc; cause<=4; status<=status&(~0x1); pc<=handle;
        push(emulator.m_csr[Csr::STATUS]);
        push(emulator.m_gpr[PC]);
        emulator.m_csr[Csr::CAUSE] = 0x00000004;
        emulator.m_csr[Csr::STATUS] = emulator.m_csr[Csr::STATUS] & ~0x00000001;
        emulator.m_gpr[PC] = emulator.m_csr[Csr::HANDLER];
        break;
      case OpCode::CALL:
        switch(curr_instr.m_mod) {
          case CallMod::CALL_PC_REL:
            // push pc; pc<=gpr[A]+gpr[B]+D;
            push(emulator.m_gpr[PC]);
            emulator.m_gpr[PC] = 
              emulator.m_gpr[curr_instr.m_reg_a] + emulator.m_gpr[curr_instr.m_reg_b] + curr_instr.m_disp;
            break;
          case CallMod::CALL_MEM_REL:
            // push pc; pc<=mem32[gpr[A]+gpr[B]+D];
            push(emulator.m_gpr[PC]);
            emulator.m_gpr[PC] = 
              readWord(emulator.m_gpr[curr_instr.m_reg_a] + emulator.m_gpr[curr_instr.m_reg_b] + curr_instr.m_disp);
            break;
        }
        break;
      case OpCode::JMP:
        switch(curr_instr.m_mod) {
          case JmpMod::JMP_PC_REL:
            // pc<=gpr[A]+D;
            emulator.m_gpr[PC] = emulator.m_gpr[curr_instr.m_reg_a] + curr_instr.m_disp;
            break;
          case JmpMod::BEQ_PC_REL:
            // if (gpr[B] == gpr[C]) pc<=gpr[A]+D;
            if (emulator.m_gpr[curr_instr.m_reg_b] == emulator.m_gpr[curr_instr.m_reg_c]) {
              emulator.m_gpr[PC] = emulator.m_gpr[curr_instr.m_reg_a] + curr_instr.m_disp;
            }
            break;
          case JmpMod::BNE_PC_REL:
            // if (gpr[B] != gpr[C]) pc<=gpr[A]+D;
            if (emulator.m_gpr[curr_instr.m_reg_b] != emulator.m_gpr[curr_instr.m_reg_c]) {
              emulator.m_gpr[PC] = emulator.m_gpr[curr_instr.m_reg_a] + curr_instr.m_disp;
            }
            break;
          case JmpMod::BGT_PC_REL:
            // if (gpr[B] signed> gpr[C]) pc<=gpr[A]+D;
            if (static_cast<int32_t>(emulator.m_gpr[curr_instr.m_reg_b]) != 
                static_cast<int32_t>(emulator.m_gpr[curr_instr.m_reg_c])) {
              emulator.m_gpr[PC] = emulator.m_gpr[curr_instr.m_reg_a] + curr_instr.m_disp;
            }
            break;
          case JmpMod::JMP_MEM_REL:
            // pc<=mem32[gpr[A]+D];
            emulator.m_gpr[PC] = readWord(emulator.m_gpr[curr_instr.m_reg_a] + curr_instr.m_disp);
            break;
          case JmpMod::BEQ_MEM_REL:
            // if (gpr[B] == gpr[C]) pc<=mem32[gpr[A]+D];
            if (emulator.m_gpr[curr_instr.m_reg_b] == emulator.m_gpr[curr_instr.m_reg_c]) {
              emulator.m_gpr[PC] = readWord(emulator.m_gpr[curr_instr.m_reg_a] + curr_instr.m_disp);
            }
            break;
          case JmpMod::BNE_MEM_REL:
            // if (gpr[B] != gpr[C]) pc<=mem32[gpr[A]+D];
            if (emulator.m_gpr[curr_instr.m_reg_b] != emulator.m_gpr[curr_instr.m_reg_c]) {
              emulator.m_gpr[PC] = readWord(emulator.m_gpr[curr_instr.m_reg_a] + curr_instr.m_disp);
            }
            break;
          case JmpMod::BGT_MEM_REL:
            // if (gpr[B] signed> gpr[C]) pc<=mem32[gpr[A]+D];
            if (static_cast<int32_t>(emulator.m_gpr[curr_instr.m_reg_b]) != 
                static_cast<int32_t>(emulator.m_gpr[curr_instr.m_reg_c])) {
              emulator.m_gpr[PC] = readWord(emulator.m_gpr[curr_instr.m_reg_a] + curr_instr.m_disp);
            }
            break;
        }
        break;
      case OpCode::XCHG:
        // temp<=gpr[B]; gpr[B]<=gpr[C]; gpr[C]<=temp;
        std::swap(emulator.m_gpr[curr_instr.m_reg_b], emulator.m_gpr[curr_instr.m_reg_c]);
        break;
      case OpCode::ARITHMETIC:
        switch(curr_instr.m_mod) {
          case ArithmeticMod::ADD:
            // gpr[A]<=gpr[B] + gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a] = 
              emulator.m_gpr[curr_instr.m_reg_b] + emulator.m_gpr[curr_instr.m_reg_c];
            break;
          case ArithmeticMod::SUB:
            // gpr[A]<=gpr[B] - gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a] = 
              emulator.m_gpr[curr_instr.m_reg_b] - emulator.m_gpr[curr_instr.m_reg_c];
            break;
          case ArithmeticMod::MUL:
            // gpr[A]<=gpr[B] * gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a] = 
              emulator.m_gpr[curr_instr.m_reg_b] * emulator.m_gpr[curr_instr.m_reg_c];
            break;
          case ArithmeticMod::DIV:
            // gpr[A]<=gpr[B] / gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a] = 
              emulator.m_gpr[curr_instr.m_reg_b] / emulator.m_gpr[curr_instr.m_reg_c];
            break;
        }
        break;
      case OpCode::LOGIC:
        switch(curr_instr.m_mod) {
          case LogicMod::NOT:
            // gpr[A]<=~gpr[B];
            emulator.m_gpr[curr_instr.m_reg_a] = ~emulator.m_gpr[curr_instr.m_reg_b];
            break;
          case LogicMod::AND:
            // gpr[A]<=gpr[B] & gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a] = 
              emulator.m_gpr[curr_instr.m_reg_b] & emulator.m_gpr[curr_instr.m_reg_c];
            break;
          case LogicMod::OR:
            // gpr[A]<=gpr[B] | gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a] = 
              emulator.m_gpr[curr_instr.m_reg_b] | emulator.m_gpr[curr_instr.m_reg_c];
            break;
          case LogicMod::XOR:
            // gpr[A]<=gpr[B] ^ gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a] = 
              emulator.m_gpr[curr_instr.m_reg_b] ^ emulator.m_gpr[curr_instr.m_reg_c];
            break;
        }
        break;
      case OpCode::SHIFT:
        switch (curr_instr.m_mod) {
          case ShiftMod::SHL:
            // gpr[A]<=gpr[B] << gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a] = 
                emulator.m_gpr[curr_instr.m_reg_b] << emulator.m_gpr[curr_instr.m_reg_c];
            break;
          case ShiftMod::SHR:
            // gpr[A]<=gpr[B] >> gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a] = 
                emulator.m_gpr[curr_instr.m_reg_b] >> emulator.m_gpr[curr_instr.m_reg_c];
            break;
        }
        break;
      case OpCode::ST:
        switch (curr_instr.m_mod) {
          case StMod::MEM_REL:
            // mem32[gpr[A]+gpr[B]+D]<=gpr[C];
            writeWord(
              emulator.m_gpr[curr_instr.m_reg_a] + emulator.m_gpr[curr_instr.m_reg_b] + curr_instr.m_disp,
              emulator.m_gpr[curr_instr.m_reg_c]
            );
            break;
          case StMod::MEM_IND_DISP:
            // gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C];
            emulator.m_gpr[curr_instr.m_reg_a]+= curr_instr.m_disp; 
            writeWord(emulator.m_gpr[curr_instr.m_reg_a], emulator.m_gpr[curr_instr.m_reg_c]);
            break;
          case StMod::MEM_IND:
            // mem32[mem32[gpr[A]+gpr[B]+D]]<=gpr[C];
            if (
              readWord(
                emulator.m_gpr[curr_instr.m_reg_a] + 
                emulator.m_gpr[curr_instr.m_reg_b] + 
                curr_instr.m_disp
              ) == term_out
              ) {
              std::cout << static_cast<char>(emulator.m_gpr[curr_instr.m_reg_c] & 0xFF);
            } else if (
              readWord(
                emulator.m_gpr[curr_instr.m_reg_a] + 
                emulator.m_gpr[curr_instr.m_reg_b] + 
                curr_instr.m_disp
              ) == tim_cfg
            ) {
              if (timer_config_map.find(emulator.m_gpr[curr_instr.m_reg_c]) != timer_config_map.end()) {
                timer_config = timer_config_map[emulator.m_gpr[curr_instr.m_reg_c]];
                // std::cout << "Tajmer konfigurisan na: " 
                //  << timer_config << " ms" << std::endl;
              } else {
                std::cerr << "Greska: Nevalidna vrednost za konfiguraciju tajmera" << std::endl;
              }
            }
            writeWord(
              readWord(emulator.m_gpr[curr_instr.m_reg_a] + emulator.m_gpr[curr_instr.m_reg_b] + curr_instr.m_disp),
              emulator.m_gpr[curr_instr.m_reg_c]
            );
            break;
          default:
            break;
        }
        break;
      case OpCode::LD:
        switch (curr_instr.m_mod) {
          case LdMod::GPR_DIR:
            // gpr[A]<=csr[B];
            emulator.m_gpr[curr_instr.m_reg_a] = emulator.m_csr[curr_instr.m_reg_b];
            break;
          case LdMod::GPR_PC_REL:
            // gpr[A]<=gpr[B]+D;
            emulator.m_gpr[curr_instr.m_reg_a] = emulator.m_gpr[curr_instr.m_reg_b] + curr_instr.m_disp;
            break;
          case LdMod::GPR_MEM_IND:
            // gpr[A]<=mem32[gpr[B]+gpr[C]+D];
            emulator.m_gpr[curr_instr.m_reg_a] = readWord(
              emulator.m_gpr[curr_instr.m_reg_b] + emulator.m_gpr[curr_instr.m_reg_c] + curr_instr.m_disp
            );
            break;
          case LdMod::GPR_MEM_IND_DISP:
            // gpr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;
            emulator.m_gpr[curr_instr.m_reg_a] = readWord(emulator.m_gpr[curr_instr.m_reg_b]);
            emulator.m_gpr[curr_instr.m_reg_b] = emulator.m_gpr[curr_instr.m_reg_b] + curr_instr.m_disp;
            break;
          case LdMod::CSR_DIR:
            // csr[A]<=gpr[B]
            emulator.m_csr[curr_instr.m_reg_a] = emulator.m_gpr[curr_instr.m_reg_b];
            // std::cout << "HANDLER = " << std::hex << emulator.m_csr[Csr::HANDLER] << std::dec << std::endl;
            break;
          case LdMod::CSR_PC_REL:
            // csr[A]<=csr[B]|D;
            emulator.m_csr[curr_instr.m_reg_a] = emulator.m_gpr[curr_instr.m_reg_b] | curr_instr.m_disp;
            break;
          case LdMod::CSR_MEM_IND:
            // csr[A]<=mem32[gpr[B]+gpr[C]+D];
            emulator.m_csr[curr_instr.m_reg_a] = readWord(
              emulator.m_gpr[curr_instr.m_reg_b] + emulator.m_gpr[curr_instr.m_reg_c] + curr_instr.m_disp
            );
            break;
          case LdMod::CSR_MEM_IND_DISP:
            // csr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;
            emulator.m_csr[curr_instr.m_reg_a] = readWord(emulator.m_gpr[curr_instr.m_reg_b]);
            emulator.m_gpr[curr_instr.m_reg_b] = emulator.m_gpr[curr_instr.m_reg_b] + curr_instr.m_disp;
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
    // std::cout << "Stigao do provere" << std::endl;
    char c = getc(stdin);
    if (c != EOF && (emulator.m_csr[Csr::STATUS] & INTERRUPT_MASK) == 0 && 
        (emulator.m_csr[Csr::STATUS] & TERMINAL_MASK) == 0) {
      emulator.m_mem32[term_in] = static_cast<uint8_t>(c);
      push(emulator.m_csr[Csr::STATUS]);
      push(emulator.m_gpr[PC]);
      emulator.m_csr[Csr::CAUSE] = 0x00000003;
      emulator.m_csr[Csr::STATUS] = emulator.m_csr[Csr::STATUS] | INTERRUPT_MASK;
      emulator.m_gpr[PC] = emulator.m_csr[Csr::HANDLER];
    }
    end_time = std::chrono::high_resolution_clock::now();
    if (timer_config != -1 && (emulator.m_csr[Csr::STATUS] & INTERRUPT_MASK) == 0 && 
        (emulator.m_csr[Csr::STATUS] & TIMER_MASK) == 0) {
      auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
      if (elapsed_time - timer_config >= -5) {
        timer_triggered = true;
      }
      if (elapsed_time >= timer_config) {
        //timer_triggered = true;
        // std::cout << "Tajmer ispaljen nakon: " << elapsed_time << " ms" << std::endl;
        push(emulator.m_csr[Csr::STATUS]);
        push(emulator.m_gpr[PC]);
        // std::cout << "Handler address: " << std::hex << emulator.m_csr[Csr::HANDLER] << std::dec << std::endl;
        // std::cout << "SP = " << std::hex << emulator.m_gpr[SP] << std::dec << std::endl;
        emulator.m_csr[Csr::CAUSE] = 0x00000002;
        emulator.m_csr[Csr::STATUS] = emulator.m_csr[Csr::STATUS] | INTERRUPT_MASK;
        emulator.m_gpr[PC] = emulator.m_csr[Csr::HANDLER];
        start_time = std::chrono::high_resolution_clock::now();
      }
    }
    // printInstruction(curr_instr.m_oc);
    // if (timer_triggered) {
    //   printInstruction(curr_instr.m_oc);
    // }
  } while(curr_instr.m_oc != OpCode::HALT);
}

int main(int argc, char* argv[]) {
  std::string input_file;

  if (handleArguments(argc, argv, input_file) != 0) {
    return 1;
  }

  if (parseHexFile(input_file) != 0) {
    return 1;
  }

  initTerminal();

  execute();

  showEmulatorState();

  return 0;
}