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

static Value parseBytevectorElement(void);

static ExprLiteral *makeNonQuotedLiteral(Value value);

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

ExprLiteral *symbol(void) {
    Value sym = OBJ_VAL(tokenToObjSymbol(&(parser.current)));
    parserAdvance();
    return makeLiteral(true, sym);
}

ExprLiteral *parseNumber(void) {
    if (!check(TOKEN_NUMBER)) {
        errorAtCurrent("Expect number.");
    }
    return parseNumberNoCheck();
}

ExprLiteral *parseBoolean(void) {
    if (!check(TOKEN_BOOLEAN)) {
        errorAtCurrent("Expect boolean.");
    }
    return parseBooleanNoCheck();
}

ExprLiteral *parseCharacter(void) {
    if (!check(TOKEN_CHARACTER)) {
        errorAtCurrent("Expect character.");
    }
    return parseCharacterNoCheck();
}

ExprLiteral *parseString(void) {
    if (!check(TOKEN_STRING)) {
        errorAtCurrent("Expect string.");
    }
    return parseStringNoCheck();
}

ExprLiteral *parseNumberNoCheck(void) {
    Value num = NUMBER_VAL(numberTokenToDouble(&(parser.current)));
    parserAdvance();
    return makeNonQuotedLiteral(num);
}

ExprLiteral *parseBooleanNoCheck(void) {
    Value b = BOOL_VAL(booleanTokenToBool(&(parser.current)));
    parserAdvance();
    return makeNonQuotedLiteral(b);
}

ExprLiteral *parseCharacterNoCheck(void) {
    Value c = CHARACTER_VAL(characterTokenToChar(&(parser.current)));
    parserAdvance();
    return makeNonQuotedLiteral(c);
}

ExprLiteral *parseStringNoCheck(void) {
    Value s = OBJ_VAL(tokenToObjString(&(parser.current)));
    parserAdvance();
    return makeNonQuotedLiteral(s);
}

ExprLiteral *parseBytevector(void) {
    return makeNonQuotedLiteral(parseVectorUsing(parseBytevectorElement));
}

static Value parseBytevectorElement(void) {
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

    return NUMBER_VAL(maybeByte);
}

ExprLiteral *parseVector(void) {
    return makeNonQuotedLiteral(parseVectorUsing(parseDatum));
}

ExprLiteral *makeLiteral(bool isQuoted, Value value) {
    ExprLiteral *literal =
        ALLOCATE_EXPR(ExprLiteral, EXPR_LITERAL, currentLocation());
    literal->isQuotation = isQuoted;
    literal->value = value;
    return literal;
}

static ExprLiteral *makeNonQuotedLiteral(Value value) {
    return makeLiteral(false, value);
}
