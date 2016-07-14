#ifndef CONSOLE_PRINT_HPP_
#define CONSOLE_PRINT_HPP_

#ifndef CMBML__ALLOW_CONSOLE_OUTPUT
#define CMBML__ALLOW_CONSOLE_OUTPUT 1
#endif  // CMBML__ALLOW_CONSOLE_OUTPUT

#ifdef CMBML__ALLOW_CONSOLE_OUTPUT
#include <cstdio>
#endif

template<typename ...Args>
void CMBML__PRINT(const char * message, ...) {
#ifdef CMBML__ALLOW_CONSOLE_OUTPUT
  va_list argptr;
  va_start(argptr, message);
  vfprintf(stdout, message, argptr);
  va_end(argptr);
#endif  // CMBML__ALLOW_CONSOLE_OUTPUT
}


#endif  // CONSOLE_PRINT_HPP_
