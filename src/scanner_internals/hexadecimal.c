#include "hexadecimal.h"

#include <ctype.h>

#include "../scanner.h"
#include "character_type_tests.h"
#include "scanner_operations.h"

void hexScalarValue() {
  while (isHexDigit(peek())) {
    advance();
  }
}

void inlineHexEscape() {
  advance();
  advance();

  hexScalarValue();

  if (!match(';')) {
    errorToken("Expect ';' after hex digits in inline hex escape.");
  }
}
