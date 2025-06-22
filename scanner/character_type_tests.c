#include "character_type_tests.h"

bool isLetter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isHexDigit(char c) { return isDigit(c) || (c >= 'a' && c <= 'f'); }

bool isSpecialInitial(char c) {
  return c == '$' || c == '%' || c == '&' || c == '*' || c == '/' || c == ':' ||
         c == '<' || c == '=' || c == '>' || c == '?' || c == '@' || c == '^' ||
         c == '_' || c == '~' || c == '!';
}

bool isInitial(char c) { return isLetter(c) || isSpecialInitial(c); }

bool isExplicitSign(char c) { return c == '+' || c == '-'; }

bool isSpecialSubsequent(char c) {
  return isExplicitSign(c) || c == '.' || c == '@';
}
bool isSubsequent(char c) {
  return isInitial(c) || isDigit(c) || isSpecialSubsequent(c);
}

bool isSignSubsequent(char c) {
  return isInitial(c) || isExplicitSign(c) || c == '@';
}

bool isDotSubsequent(char c) { return c == '.' || isSignSubsequent(c); }

bool isIntralineWhitespace(char c) { return c == ' ' || c == '\t'; }
