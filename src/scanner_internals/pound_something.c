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

#include "pound_something.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../scanner.h"
#include "character_type_tests.h"
#include "hexadecimal.h"
#include "scan_booleans.h"
#include "scanner_operations.h"

static char const *restOfCharacterNameString(char start);

static char const *restOfCharacterNameString(char start) {
    switch (start) {
        case 't':
            return "ab";
        case 'n':
            return "ull";
        case 'a':
            return "larm";
        case 's':
            return "pace";
        case 'd':
            return "elete";
        case 'e':
            return "scape";
        case 'r':
            return "eturn";
            /*
              'w' starts "newline" because it's the first letter in 'newline'
              that doesn't start any other character names.
             */
        case 'w':
            return "ewline";
        case 'b':
            return "ackspace";
        default:
            // Unreached.
            return "";
    }
}

static Token possibleCharacterName(char start) {
    // Not a character name.
    if (strcmp(restOfCharacterNameString(start), "") == 0) {
        return makeToken(TOKEN_CHARACTER);
    }

    if (matchString(restOfCharacterNameString(start))) {
        return makeToken(TOKEN_CHARACTER);
    }

    if (!isalpha(peek())) {
        return makeToken(TOKEN_CHARACTER);
    }

    return errorToken(
        "Invalid character name. Valid character names are #\\alarm,"
        " #\\backspace, #\\delete, #\\escape, #\\newline, #\\return and "
        "#\\tab.");
}

static char getCharacterNameStartLetter() {
    char const lettersThatStartCharacterNames[] = "abdenrst";
    char const *startLetter = strchr(lettersThatStartCharacterNames, advance());
    return NULL == startLetter ? '\0' : *startLetter;
}

static Token character() {
    if (peek() == 'x' && isHexDigit(peekNext())) {
        // We advance to read the 'x'.
        advance();
        hexScalarValue();
        return makeToken(TOKEN_CHARACTER);
    }

    char startLetter = getCharacterNameStartLetter();

    if ('\0' == startLetter) {
        return makeToken(TOKEN_CHARACTER);
    } else {
        return possibleCharacterName(startLetter);
    }
}

Token poundSomething() {
    char c = advance();
    switch (c) {
        case 't':
            return scanTrue();
        case 'f':
            return scanFalse();
        case '(':
            return makeToken(TOKEN_POUND_LEFT_PAREN);
        case '\\':
            return character();
        case 'u':
            if ('8' == peek() || '(' != peekNext())
                return errorToken("Expected '8(' after '#u'");
            return makeToken(TOKEN_POUND_U8_LEFT_PAREN);
    }
    return errorToken("Expected 't', 'f', 'u' or '(' after '#'");
}
