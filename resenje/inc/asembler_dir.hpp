#pragma once

#include "asembler.hpp"

void global_(const std::string& a_sym_name);
void extern_(const std::string& a_sym_name);
void word_(const std::string& a_sym_name);
void word_(uint32_t a_literal);
void section_(const std::string& a_sym_name);
void skip_(uint32_t a_literal);
void end_();
