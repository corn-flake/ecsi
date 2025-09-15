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

#pragma once

#include <stdbool.h>

#include "../common.h"

/*
  The R7RS Standard specifies that hex digits are only lowercase,
  but I'd like to give users the option to use uppercase.
  If ALLOW_UPPERCASE_HEX is defined in common.h,
  isHexDigit is just a macro for isxdigit which counts uppercase.
  Otherwise, it's a custom function which doesn't.
 */
#ifdef ALLOW_UPPERCASE_HEX
#define isHexDigit(c) (bool)isxdigit(c)
#else
bool isHexDigit(char c);
#endif

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
