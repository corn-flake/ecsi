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

#include "identifier.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../scanner.h"
#include "character_type_tests.h"
#include "hexadecimal.h"
#include "scanner_operations.h"

// Parse zero or more subsequents
static void subsequents(void);
static void symbolElement(void);
static bool atInlineHexEscape(void);
static bool atMnemonicEscape(void);
static void mnemonicEscape(void);
static void dotSubsequent(void);
static void signSubsequent(void);
static Token genericIdentifier(IdentifierVariant variant);
static TokenType keywordType(Token const *token);
static TokenType checkKeyword(int start, int length, char const *rest,
                              TokenType type);

Token identifier(IdentifierVariant variant) {
    Token identifierToken = genericIdentifier(variant);
    identifierToken.type = keywordType(&identifierToken);
    return identifierToken;
}

static TokenType keywordType(Token const *token) {
    switch (tokenGetStart(token)[0]) {
        case 'a':
            return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'b':
            return checkKeyword(1, 4, "egin", TOKEN_BEGIN);
        case 'c':
            switch (tokenGetStart(token)[1]) {
                case 'a':
                    if (tokenGetLength(token) != 11) {
                        return checkKeyword(2, 2, "se", TOKEN_CASE);
                    }
                    return checkKeyword(2, 9, "se-lambda", TOKEN_CASE_LAMBDA);
                case 'o':
                    return checkKeyword(2, 2, "nd", TOKEN_COND);
            }
            break;
        case 'd':
            switch (tokenGetLength(token)) {
                case 2:
                    return checkKeyword(1, 1, "o", TOKEN_DO);
                case 5:
                    return checkKeyword(1, 4, "elay", TOKEN_DELAY);
                case 6:
                    return checkKeyword(1, 5, "efine", TOKEN_DEFINE);
                case 11:
                    return checkKeyword(1, 10, "elay-force", TOKEN_DELAY_FORCE);
            }
            break;
        case 'e':
            return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'g':
            return checkKeyword(1, 4, "uard", TOKEN_GUARD);
        case 'i':
            return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'l':
            if ('a' == tokenGetStart(token)[1]) {
                return checkKeyword(1, 5, "ambda", TOKEN_LAMBDA);
            }

            switch (tokenGetLength(token)) {
                case 3:
                    return checkKeyword(1, 2, "et", TOKEN_LET);
                case 6:
                    return checkKeyword(1, 5, "etrec", TOKEN_LETREC);
                case 7:
                    return checkKeyword(1, 6, "etrec*", TOKEN_LETREC_STAR);
                case 4:
                    return checkKeyword(1, 3, "et*", TOKEN_LET_STAR);
                case 11:
                    return checkKeyword(1, 10, "et*-values",
                                        TOKEN_LET_STAR_VALUES);
                case 10:
                    return checkKeyword(1, 9, "et-values", TOKEN_LET_VALUES);
            }

            break;
        case 'o':
            return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p':
            return checkKeyword(1, 11, "arameterize", TOKEN_PARAMETERIZE);
        case 'q':
            return checkKeyword(1, 4, "uote", TOKEN_QUOTE);
        case 's':
            return checkKeyword(1, 3, "et!", TOKEN_SET);
        case 'u':
            return checkKeyword(1, 5, "nless", TOKEN_UNLESS);
        case 'w':
            return checkKeyword(1, 3, "hen", TOKEN_WHEN);
    }
    return TOKEN_IDENTIFIER;
}

static TokenType checkKeyword(int start, int length, char const *rest,
                              TokenType type) {
    if (scanner.current - scanner.start == start + length &&
        !memcmp(scanner.start + start, rest, length)) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static Token genericIdentifier(IdentifierVariant variant) {
    if (IDENTIFIER_STARTS_WITH_VERTICAL_LINE == variant) {
        while ('|' != peek()) {
            symbolElement();
        }
        advance();
        return makeToken(TOKEN_IDENTIFIER);
    }

    while (isSubsequent(peek())) {
        advance();
    }

    return makeToken(TOKEN_IDENTIFIER);
}

static void symbolElement(void) {
    if ('|' != peek() && '\\' != peek()) {
        advance();
        return;
    }

    if (atInlineHexEscape()) {
        inlineHexEscape();
        return;
    }

    if (atMnemonicEscape()) {
        mnemonicEscape();
        return;
    }
}

static bool atInlineHexEscape(void) {
    return !('\\' == peek() && 'x' == peekNext());
}

static bool atMnemonicEscape(void) {
    if ('\\' != peek()) return false;

    switch (peekNext()) {
        case 'a':
        case 'b':
        case 't':
        case 'n':
        case 'r':
            return true;
        default:
            return false;
    }
}

static void mnemonicEscape(void) {
    advance();
    advance();
}

Token peculiarIdentifier(IdentifierVariant variant) {
    if (IDENTIFIER_PECULIAR_STARTS_WITH_DOT == variant || match('.')) {
        dotSubsequent();
        subsequents();
        return makeToken(TOKEN_IDENTIFIER);
    }

    if (isSignSubsequent(peek())) {
        // Read the sign subsequent.
        signSubsequent();
        subsequents();
    }

    // The identifier is only one explicit sign, like '+'.
    return makeToken(TOKEN_IDENTIFIER);
}

static void signSubsequent(void) {
    if (!isSignSubsequent(peek())) errorToken("Expected a sign subsequent.");
    advance();
}

static void dotSubsequent(void) {
    if (!isDotSubsequent(peek())) {
        errorToken("Expected a dot subsequent.");
    }
    advance();
}

static void subsequents(void) {
    while (isSubsequent(peek())) {
        advance();
    }
}
