#include "literals.h"

#include "../object.h"
#include "parser.h"
#include "token_to_type.h"

Value vector(bool parseDatums) {
  ObjVector *vector = newVector();
  while (parser.current->type != TOKEN_RIGHT_PAREN) {
    vectorAppend(vector, parseDatums ? parseDatum() : parseExpression());
  }
  return OBJ_VAL(vector);
}

Value symbol() {
  Value sym =
      OBJ_VAL(newSymbol(parser.current->start, (int)parser.current->length));
  parserAdvance();
  return sym;
}

Value bytevector() {
  parserAdvance();
  ObjVector *bytevector = newVector();

  while (parser.current->type != TOKEN_RIGHT_PAREN) {
    if (parser.current->type != TOKEN_NUMBER) {
      errorAtCurrent(
          "Members of bytevector must be numbers between 0 "
          "and 255 inclusive.");
      break;
    }

    double maybeByte = numberTokenToDouble(parser.current);
    if (((double)(long)maybeByte) != maybeByte) {
      errorAtCurrent("Members of bytevector must be exact integers.");
    }

    int byte = (int)maybeByte;

    if (byte > 255 || byte < 0) {
      errorAtCurrent(
          "Members of bytevector must be exact integers between 0 "
          "and 255 inclusive.");
      break;
    }
    vectorAppend(bytevector, NUMBER_VAL((double)byte));
  }
  return OBJ_VAL(bytevector);
}
