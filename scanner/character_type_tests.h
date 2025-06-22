#pragma once

#include <stdbool.h>

/*
  Returns true if c is a letter, uppercase or lowercase, otherwise returns
  false.
 */
bool isLetter(char c);

/*
  Returns true if c is a number, otherwise returns false.
 */
bool isDigit(char c);

/*
  Returns true if c is a number or 'a', 'b', 'c', 'd', 'e', or 'f', otherwise
  returns false.
 */
bool isHexDigit(char c);

bool isSpecialInitial(char c);
bool isInitial(char c);

/*
  Returns true if c is '+' or '-', otherwise returns false.
 */
bool isExplicitSign(char c);
bool isSpecialSubsequent(char c);
bool isSubsequent(char c);
bool isSignSubsequent(char c);
bool isDotSubsequent(char c);
bool isIntralineWhitespace(char c);
