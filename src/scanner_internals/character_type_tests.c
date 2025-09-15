/*
  Copyright 2025 Evan Cooney

  This file is part of Ecsi.

  Ecsi is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  Ecsi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  Ecsi. If not, see <https://www.gnu.org/licenses/>.
 */

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
