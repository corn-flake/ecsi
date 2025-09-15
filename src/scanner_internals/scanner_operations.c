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

#include "scanner_operations.h"

char peek() { return isAtEnd() ? '\0' : *scanner.current; }

char peekNext() { return isAtEnd() ? '\0' : scanner.current[1]; }

char advance() {
    scanner.current++;
    return scanner.current[-1];
}

bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;

    scanner.current++;
    return true;
}

bool matchString(char const *string) {
    size_t charsLeft = strlen(scanner.current);
    size_t stringLength = strlen(string);

    if (stringLength > charsLeft) return false;
    if (strncmp(string, scanner.current, stringLength) != 0) return false;

    for (size_t i = 0; i < stringLength; i++) advance();

    return true;
}

Token makeToken(TokenType type) {
    Token token = {
        .type = type,
        .start = scanner.start,
        .length = scanner.current - scanner.start,
        .line = scanner.line,
    };
    return token;
}

Token errorToken(char const *message) {
    Token token = {
        .type = TOKEN_ERROR,
        .start = message,
        .length = strlen(message),
        .line = scanner.line,
    };
    return token;
}

bool isAtEnd() { return *scanner.current == '\0'; }
