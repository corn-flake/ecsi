#include "hexadecimal.h"

#include "character_type_tests.h"
#include "scanner.h"

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
