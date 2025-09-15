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

#include "../object.h"
#include "../parser.h"
#include "parser_operations.h"
#include "token_to_type.h"

static Value parseBytevectorElement();

Value parseVectorUsing(ParseFn parse) {
    ObjVector *vector = newVector();
    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        return OBJ_VAL(vector);
    }

    while (canContinueList()) {
        vectorAppend(vector, parse());
    }

    if (!parserMatch(TOKEN_RIGHT_PAREN)) {
        errorAtCurrent("Expect ')' to close vector literal.");
    }

    return OBJ_VAL(vector);
}

Value symbol() {
    Value sym = OBJ_VAL(tokenToObjSymbol(parser.current));
    parserAdvance();
    return sym;
}

Value parseNumber() {
    if (!check(TOKEN_NUMBER)) {
        errorAtCurrent("Expect number.");
    }
    return parseNumberNoCheck();
}

Value parseBoolean() {
    if (!check(TOKEN_BOOLEAN)) {
        errorAtCurrent("Expect boolean.");
    }
    return parseBooleanNoCheck();
}

Value parseCharacter() {
    if (!check(TOKEN_CHARACTER)) {
        errorAtCurrent("Expect character.");
    }
    return parseCharacterNoCheck();
}

Value parseString() {
    if (!check(TOKEN_STRING)) {
        errorAtCurrent("Expect string.");
    }
    return parseStringNoCheck();
}

Value parseNumberNoCheck() {
    Value num = NUMBER_VAL(numberTokenToDouble(parser.current));
    parserAdvance();
    return num;
}

Value parseBooleanNoCheck() {
    Value b = BOOL_VAL(booleanTokenToBool(parser.current));
    parserAdvance();
    return b;
}
Value parseCharacterNoCheck() {
    Value c = CHARACTER_VAL(characterTokenToChar(parser.current));
    parserAdvance();
    return c;
}
Value parseStringNoCheck() {
    Value s = OBJ_VAL(tokenToObjString(parser.current));
    parserAdvance();
    return s;
}

Value parseBytevector() { return parseVectorUsing(parseBytevectorElement); }

static Value parseBytevectorElement() {
    if (!parserMatch(TOKEN_NUMBER)) {
        errorAtCurrent(
            "Members of bytevector must be numbers between 0 "
            "and 255 inclusive.");
    }

    double maybeByte = numberTokenToDouble(parser.previous);
    if (!IS_EXACT_INTEGER(NUMBER_VAL(maybeByte))) {
        error("Members of bytevector must be exact integers.");
    }

    int byte = (int)maybeByte;

    if (byte > 255 || byte < 0) {
        error(
            "Members of bytevector must be exact integers between 0 "
            "and 255 inclusive.");
    }

    return NUMBER_VAL(maybeByte);
}
