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

#include <stdarg.h>
#include <stdbool.h>

#include "../scanner.h"
#include "../value.h"

typedef Value (*ParseFn)(void);

void formattedErrorAt(Token const *token, char const *format, ...);
void varArgsFormattedErrorAt(Token const *token, char const *format,
                             va_list args);
void formattedError(char const *format, ...);
void formattedErrorAtCurrent(char const *format, ...);

void errorAt(Token const *token, char const *message);
void error(char const *message);
void errorAtCurrent(char const *message);
void consume(TokenType type, char const *message);

void parserAdvance();
bool parserMatch(TokenType type);
bool matchToken(Token const *token);
bool check(TokenType type);
bool checkToken(Token const *token);
bool parserIsAtEnd();

bool canContinueList();
bool tokenMatchesString(Token *token, char *const string);
bool previousTokenMatchesString(char *const string);
bool currentTokenMatchesString(char *const string);

Value parseListUsing(ParseFn parse);
// n == -1 indicates to parse until a right paren is found.
Value parseNExprsIntoList(ParseFn parse, int n);

// n is unsigned because 'parse list of at least -1' makes no sense.
Value parseAtLeastNExprsUsing(ParseFn parse, size_t n);
Value parseListOfExpressions();
