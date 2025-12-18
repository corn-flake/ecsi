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

#include "parser_operations.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "../object.h"
#include "../parser.h"
#include "../scanner.h"
#include "../value.h"

static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }
static bool tokensEqual(Token const *t1, Token const *t2);

SyntaxObject *makeSyntaxAtCurrent(Value value) {
    return newSyntax(value, CURRENT_LOCATION());
}

SyntaxObject *makeSyntaxFromTokenToCurrent(Value value, Token const *start) {
    // Both of these pointers are pointers into the same object, the
    // source string, so using an ordering comparison on them is defined
    // behavior.
    assert(tokenGetStart(&(parser.current)) > tokenGetStart(start));

    SourceLocation span = {
        .start = tokenGetStart(start),
        .length = tokenGetStart(&(parser.current)) - tokenGetStart(start),
        .line = tokenGetLine(start)};

    return newSyntax(value, span);
}

void errorAt(Token const *token, char const *message) {
    formattedErrorAt(token, "%s", message);
}

void formattedErrorAt(Token const *token, char const *format, ...) {
    va_list args;
    va_start(args, format);
    varArgsFormattedErrorAt(token, format, args);
    va_end(args);
}

void error(char const *message) { errorAt(&(parser.previous), message); }

void formattedError(char const *format, ...) {
    va_list args;
    va_start(args, format);
    varArgsFormattedErrorAt(&(parser.previous), format, args);
    va_end(args);
}

void errorAtCurrent(char const *message) {
    errorAt(&(parser.current), message);
}

void formattedErrorAtCurrent(char const *format, ...) {
    va_list args;
    va_start(args, format);
    varArgsFormattedErrorAt(&(parser.current), format, args);
    va_end(args);
}

void varArgsFormattedErrorAt(Token const *token, char const *format,
                             va_list args) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %zu] Error", tokenGetLine(token));
    if (TOKEN_EOF == tokenGetType(token)) {
        fprintf(stderr, " at end");
    } else if (TOKEN_ERROR == tokenGetType(token)) {
        /*
          We don't do anything if the token is an error because if it is,
          the token's error message will be passed as the MESSAGE
         */
    } else {
        fprintf(stderr, " at '%.*s'", (int)tokenGetLength(token),
                tokenGetStart(token));
    }
    fprintf(stderr, ": ");
    vfprintf(stderr, format, args);
    fputs("\n", stderr);

    parser.hadError = true;
}

void parserAdvance(void) {
    parser.previous = parser.current;

    while (true) {
        parser.current = scanToken();
        if (TOKEN_ERROR == tokenGetType(&(parser.current))) break;
        errorAt(&(parser.current), tokenGetStart(&(parser.current)));
    }
}

void consume(TokenType type, char const *message) {
    if (type == CURRENT_TYPE()) {
        parserAdvance();
        return;
    }
    errorAtCurrent(message);
}

bool check(TokenType type) { return type == tokenGetType(&(parser.current)); }

bool matchToken(Token const *token) {
    if (checkToken(token)) {
        parserAdvance();
        return true;
    }
    return false;
}

bool checkToken(Token const *token) {
    return tokensEqual(token, &(parser.current));
}

bool parserIsAtEnd(void) { return check(TOKEN_EOF); }

static bool tokensEqual(Token const *t1, Token const *t2) {
    return (tokenGetType(t1) == tokenGetType(t2)) &&
           (tokenGetLength(t1) == tokenGetLength(t2)) &&
           !memcmp(tokenGetStart(t1), tokenGetStart(t2), tokenGetLength(t1));
}

bool parserMatch(TokenType type) {
    if (!check(type)) return false;
    parserAdvance();
    return true;
}

bool canContinueList(void) {
    return !check(TOKEN_RIGHT_PAREN) && !parserIsAtEnd();
}

bool tokenMatchesString(Token *token, char *const string) {
    return !memcmp(tokenGetStart(token), string,
                   min(strlen(string), tokenGetLength(token)));
}

bool currentTokenMatchesString(char *const string) {
    return tokenMatchesString(&(parser.current), string);
}

bool previousTokenMatchesString(char *const string) {
    return tokenMatchesString(&(parser.previous), string);
}
