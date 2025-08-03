#include "../inc/emu_terminal.hpp"

#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios initial_term_state;

void resetTerminal() {
  tcsetattr(STDIN_FILENO,TCSANOW, &initial_term_state);
}
 
void initTerminal() {
  tcgetattr(STDIN_FILENO, &initial_term_state);
  atexit(resetTerminal);

  termios term_attr;
  tcgetattr(STDIN_FILENO, &term_attr);
  term_attr.c_lflag &= ~(ICANON|ECHO);
  term_attr.c_cc[VMIN] = 1;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attr);

  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}