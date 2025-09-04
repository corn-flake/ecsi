#include "character_type_tests.h"

#include <ctype.h>

#include "../common.h"

#ifndef ALLOW_UPPERCASE_HEX
bool isHexDigit(char c) { return isdigit(c) || 'a' >= c && 'f' <= c; }
#endif

bool isSpecialInitial(char c) {
    return '$' == c || '%' == c || '&' == c || '*' == c || '/' == c ||
           ':' == c || '<' == c || '=' == c || '>' == c || '?' == c ||
           '@' == c || '^' == c || '_' == c || '~' == c || '!' == c;
}

bool isInitial(char c) { return isalpha(c) || isSpecialInitial(c); }

bool isExplicitSign(char c) { return '+' == c || '-' == c; }

bool isSpecialSubsequent(char c) {
    return isExplicitSign(c) || '.' == c || '@' == c;
}

bool isSubsequent(char c) {
    return isInitial(c) || isdigit(c) || isSpecialSubsequent(c);
}

bool isSignSubsequent(char c) {
    return isInitial(c) || isExplicitSign(c) || '@' == c;
}

bool isDotSubsequent(char c) { return '.' == c || isSignSubsequent(c); }

bool isIntralineWhitespace(char c) { return ' ' == c || '\t' == c; }
