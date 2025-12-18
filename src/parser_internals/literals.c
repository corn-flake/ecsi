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

#include "literals.h"

#include <assert.h>

#include "../object.h"
#include "../parser.h"
#include "parser_operations.h"
#include "scanner.h"
#include "token_to_type.h"
#include "value.h"

static SyntaxObject *parseBytevectorElement(void);
static SyntaxObject *parseVectorUsing(ParseDatumFn parse);

SyntaxObject *symbol(void) {
    assert(tokenIsKeyword(&(parser.current)) || check(TOKEN_IDENTIFIER));
    Value sym = OBJ_VAL(tokenToObjSymbol(&(parser.current)));
    parserAdvance();
    return makeSyntaxAtCurrent(sym);
}

SyntaxObject *parseNumber(void) {
    assert(check(TOKEN_NUMBER));
    Value num = NUMBER_VAL(numberTokenToDouble(&(parser.current)));
    parserAdvance();
    return makeSyntaxAtCurrent(num);
}

SyntaxObject *parseBoolean(void) {
    assert(check(TOKEN_BOOLEAN));
    Value boolean = BOOL_VAL(booleanTokenToBool(&(parser.current)));
    parserAdvance();
    return makeSyntaxAtCurrent(boolean);
}

SyntaxObject *parseCharacter(void) {
    assert(check(TOKEN_CHARACTER));
    Value character = CHARACTER_VAL(characterTokenToChar(&(parser.current)));
    parserAdvance();
    return makeSyntaxAtCurrent(character);
}

SyntaxObject *parseString(void) {
    assert(check(TOKEN_CHARACTER));
    Value string = OBJ_VAL(tokenToObjSymbol(&(parser.current)));
    parserAdvance();
    return makeSyntaxAtCurrent(string);
}

SyntaxObject *parseBytevector(void) {
    return parseVectorUsing(parseBytevectorElement);
}

static SyntaxObject *parseBytevectorElement(void) {
    if (!parserMatch(TOKEN_NUMBER)) {
        errorAtCurrent(
            "Members of bytevector must be numbers between 0 "
            "and 255 inclusive.");
    }

    double maybeByte = numberTokenToDouble(&(parser.previous));
    if (!IS_EXACT_INTEGER(NUMBER_VAL(maybeByte))) {
        error("Members of bytevector must be exact integers.");
    }

    int byte = (int)maybeByte;

    if (byte > 255 || byte < 0) {
        error(
            "Members of bytevector must be exact integers between 0 "
            "and 255 inclusive.");
    }

    return makeSyntaxAtCurrent(NUMBER_VAL(maybeByte));
}

SyntaxObject *parseVector(void) { return parseVectorUsing(parseExpression); }

static SyntaxObject *parseVectorUsing(ParseDatumFn parse) {
    assert(check(TOKEN_POUND_LEFT_PAREN) || check(TOKEN_POUND_U8_LEFT_PAREN));
    Token const *vectorStart = &(parser.current);
    parserAdvance();

    ObjVector *vector = newVector();

    while (canContinueList()) {
        vectorAppend(vector, OBJ_VAL(parse()));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close vector.");

    return makeSyntaxFromTokenToCurrent(OBJ_VAL(vector), vectorStart);
}
