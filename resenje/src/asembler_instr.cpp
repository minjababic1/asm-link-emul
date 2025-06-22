#include <cstdint>

/// @brief Operation codes
constexpr uint8_t HALT_OC = 0x00;
constexpr uint8_t INT_OC = 0x01;
constexpr uint8_t CALL_OC = 0x02;
constexpr uint8_t JMP_OC = 0x03;
constexpr uint8_t XCHNG_OC = 0x04;
constexpr uint8_t AR_OC = 0x05;
constexpr uint8_t LOG_OC = 0x06;
constexpr uint8_t SH_OC = 0x07;
constexpr uint8_t ST_OC = 0x08;
constexpr uint8_t LD_OC = 0x09;

/// @brief Call instruction modificators
constexpr uint8_t CALL_IND = 0x00;
constexpr uint8_t CALL_MEM_IND = 0x01;

/// @brief Jmp instruction modificators
constexpr uint8_t JMP