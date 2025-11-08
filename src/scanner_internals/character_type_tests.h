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

// Determine whether c is a "special inital" character (see R7Rs standard).
bool isSpecialInitial(char c);

// Determine whether c is an "initial" character (see R7Rs standard).
bool isInitial(char c);

/*
  Returns true if c is '+' or '-', otherwise returns false.
  This is the definition of an "explicit sign" character in the
  R7Rs standard.
 */
bool isExplicitSign(char c);

// Determine whether c is a "special subsequent" (see R7Rs standard).
bool isSpecialSubsequent(char c);

// Determine whether c is a "subsequent" (see R7Rs standard).
bool isSubsequent(char c);

// Determine whether c is a "sign subsequent" (see R7Rs standard).
bool isSignSubsequent(char c);

// Determine whether c is a "dot subsequent" (see R7Rs standard).
bool isDotSubsequent(char c);

// Determine whether c is an "intraline whitespace" (see R7Rs standard).
bool isIntralineWhitespace(char c);
