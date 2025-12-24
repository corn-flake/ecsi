/*
  Copyright 2025 Evan Cooney
  Copyright 2015-2020 Robert Nystrom

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

#include "scanner.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "object.h"
#include "scanner_internals/character_type_tests.h"
#include "scanner_internals/identifier.h"
#include "scanner_internals/intertoken_space.h"
#include "scanner_internals/pound_something.h"
#include "scanner_internals/scanner_operations.h"

Scanner scanner;

// Scan and return a number token.
static Token number(void);

// Scan and return a string token.
static Token string(void);

char const *tokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_IDENTIFIER:
            return "TOKEN_IDENTIFIER";
        case TOKEN_NUMBER:
            return "TOKEN_NUMBER";
        case TOKEN_STRING:
            return "TOKEN_STRING";
        case TOKEN_LEFT_PAREN:
            return "TOKEN_LEFT_PAREN";
        case TOKEN_RIGHT_PAREN:
            return "TOKEN_RIGHT_PAREN";
        case TOKEN_ERROR:
            return "TOKEN_ERROR";
        case TOKEN_PERIOD:
            return "TOKEN_PERIOD";
        case TOKEN_COMMA:
            return "TOKEN_COMMA";
        case TOKEN_BACKQUOTE:
            return "TOKEN_BACKQUOTE";
        case TOKEN_COMMA_AT:
            return "TOKEN_COMMA_AT";
        case TOKEN_BOOLEAN:
            return "TOKEN_BOOLEAN";
        case TOKEN_POUND_LEFT_PAREN:
            return "TOKEN_POUND_LEFT_PAREN";
        case TOKEN_POUND_U8_LEFT_PAREN:
            return "TOKEN_POUND_U8_LEFT_PAREN";
        case TOKEN_CHARACTER:
            return "TOKEN_CHARACTER";
        case TOKEN_EOF:
            return "TOKEN_EOF";
        default:
            UNREACHABLE();
    }
}

bool textOfTokenEqualToString(Token const *token, char const *string) {
    return strncmp(tokenGetStart(token), string, tokenGetLength(token));
}

bool tokenIsKeyword(Token const *token) {
    return TOKEN_AND <= tokenGetType(token) &&
           tokenGetType(token) <= TOKEN_WHEN;
}

void printToken(Token const *token) {
    printf("Token { .type = %s, .start = '%.*s', .line = %zu }\n",
           tokenTypeToString(tokenGetType(token)), (int)tokenGetLength(token),
           tokenGetStart(token), tokenGetLine(token));
}

ObjString *tokenToObjString(Token const *token) {
    return copyString(tokenGetStart(token), tokenGetLength(token));
}

ObjSymbol *tokenToObjSymbol(Token const *token) {
    return newSymbol(tokenGetStart(token), tokenGetLength(token));
}

void initScanner(char const *source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static Token number(void) {
    while (isdigit(peek())) advance();

    return makeToken(TOKEN_NUMBER);
}

static Token string(void) {
    while (peek() != '"' && !isAtEnd()) {
        if ('\n' == peek()) scanner.line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    // The closing quote
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken(void) {
    skipIntertokenSpace();
    scanner.start = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    if (isdigit(c)) return number();

    if (isInitial(c)) return identifier(IDENTIFIER_NO_VERTICAL_LINE);

    if (isExplicitSign(c)) {
        return peculiarIdentifier(IDENTIFIER_PECULIAR_NO_DOT);
    }

    switch (c) {
        case '(':
            return makeToken(TOKEN_LEFT_PAREN);
        case ')':
            return makeToken(TOKEN_RIGHT_PAREN);
        case '\'':
            return makeToken(TOKEN_QUOTE);
        case '`':
            return makeToken(TOKEN_BACKQUOTE);
        case ',':
            if (match('@')) {
                return makeToken(TOKEN_COMMA_AT);
            }
            return makeToken(TOKEN_COMMA);
        case '.':
            if (isDotSubsequent(peek())) {
                return peculiarIdentifier(IDENTIFIER_PECULIAR_STARTS_WITH_DOT);
            }
            return makeToken(TOKEN_PERIOD);
        case '"':
            return string();
        case '|':
            return identifier(IDENTIFIER_STARTS_WITH_VERTICAL_LINE);
        case '#':
            return poundSomething();
    }

    return errorToken("Unexpected character.");
}

size_t tokenGetLine(Token const *token) { return token->location.line; }

size_t tokenGetLength(Token const *token) { return token->location.length; }

char const *tokenGetStart(Token const *token) { return token->location.start; }

TokenType tokenGetType(Token const *token) { return token->type; }
