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
    if (!parserMatch(TOKEN_NUMBER)) {
        errorAtCurrent("Expect number.");
    }
    return parseNumberNoCheck();
}

Value parseBoolean() {
    if (!parserMatch(TOKEN_BOOLEAN)) {
        errorAtCurrent("Expect boolean.");
    }
    return parseBooleanNoCheck();
}

Value parseCharacter() {
    if (!parserMatch(TOKEN_CHARACTER)) {
        errorAtCurrent("Expect character.");
    }
    return parseCharacterNoCheck();
}

Value parseString() {
    if (!parserMatch(TOKEN_STRING)) {
        errorAtCurrent("Expect string.");
    }
    return parseStringNoCheck();
}

Value parseNumberNoCheck() {
    return NUMBER_VAL(numberTokenToDouble(parser.current));
}

Value parseBooleanNoCheck() {
    return BOOL_VAL(booleanTokenToBool(parser.current));
}
Value parseCharacterNoCheck() {
    return CHARACTER_VAL(characterTokenToChar(parser.current));
}
Value parseStringNoCheck() { return OBJ_VAL(tokenToObjString(parser.current)); }

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
