#include "parser.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../memory.h"
#include "../object.h"
#include "../value.h"
#include "../vm.h"
#include "derived_expressions.h"
#include "literals.h"
#include "parser.h"
#include "token_to_type.h"

static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }

Parser parser;

void errorAt(Token *token, const char *message) {
  if (parser.panicMode) return;
  parser.panicMode = true;
  fprintf(stderr, "[line %zu] Error", token->line);
  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    /*
      We don't do anything if the token is an error because if it is,
      the token's error message will be passed as the MESSAGE
     */
  } else {
    fprintf(stderr, " at '%.*s'", (int)token->length, token->start);
  }
  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

void error(const char *message) { errorAt(parser.previous, message); }

void errorAtCurrent(const char *message) { errorAt(parser.current, message); }

/*
  We call this function parserAdvance to avoid name conflict
  with the scanner's advance function.
 */
void parserAdvance() {
  parser.previous = parser.current;

  while (true) {
    parser.current++;
    if (parser.current->type != TOKEN_ERROR) break;
    errorAt(parser.current, parser.current->start);
  }
}

void consume(TokenType type, const char *message) {
  if (parser.current->type == type) {
    parserAdvance();
    return;
  }
  errorAtCurrent(message);
}

bool check(TokenType type) { return parser.current->type == type; }

bool parserMatch(TokenType type) {
  if (!check(type)) return false;
  parserAdvance();
  return true;
}

void initParser(TokenArray tokens) {
  parser.tokens = tokens;
  parser.previous = tokens.array;
  parser.current = tokens.array;
  parser.hadError = false;
  parser.panicMode = false;
  parser.ast = NIL_VAL;
}

// We forward declare this to allow us to write mutally recursive functions.
Value parseDatum();

Value parseList(bool parseDatums) {
  if (check(TOKEN_RIGHT_PAREN)) return NIL_VAL;

  Value firstElement = parseDatums ? parseDatum() : parseExpression();

  push(firstElement);
  ObjPair *list = newPair(firstElement, NIL_VAL);
  pop();  // firstElement

  push(OBJ_VAL(list));
  while (!check(TOKEN_RIGHT_PAREN)) {
    append(list, parseDatums ? parseDatum() : parseExpression());
  }
  pop();  // list

  return OBJ_VAL(list);
}

/*
static void addToAst(Value value) {
  assert(IS_NIL(parser.ast) || IS_PAIR(parser.ast));
  push(value);
  if (IS_NIL(parser.ast)) {
    parser.ast = CONS(value, NIL_VAL);
  } else {
    append(AS_PAIR(parser.ast), value);
  }
  pop();  // value
}
*/

Value parseDatum() {
  Value value = NIL_VAL;

  switch (parser.current->type) {
      // Simple datums.
    case TOKEN_BOOLEAN:
      value = BOOL_VAL(booleanTokenToBool(parser.current));
      parserAdvance();
      break;
    case TOKEN_NUMBER:
      value = NUMBER_VAL(numberTokenToDouble(parser.current));
      parserAdvance();
      break;
    case TOKEN_CHARACTER:
      value = CHARACTER_VAL(characterTokenToChar(parser.current));
      parserAdvance();
      break;
    case TOKEN_STRING:
      value = OBJ_VAL(
          copyString(parser.current->start, (int)parser.current->length));
      parserAdvance();
      break;
    case TOKEN_IDENTIFIER:
      value = symbol();
      parserAdvance();
      break;
    case TOKEN_POUND_U8_LEFT_PAREN:
      value = bytevector();
      break;

      // Compound datums.
    case TOKEN_LEFT_PAREN:
      // Consume the '('
      parserAdvance();
      value = parseList(true);
      break;
    case TOKEN_POUND_LEFT_PAREN:
      // Consume the '#('
      parserAdvance();
      value = vector(true);
      break;
    case TOKEN_QUOTE:
    case TOKEN_BACKQUOTE:
    case TOKEN_COMMA:
    case TOKEN_COMMA_AT:
      parserAdvance();
      value = parseDatum();
      break;
    default:
      value = NIL_VAL;
  }

  return value;
}

void markParserRoots() { markValue(parser.ast); }

static bool atLambda() {
  return memcmp(parser.current->start, "lambda",
                min(6, parser.current->length)) == 0;
}

static Value lambda_() {
  Value lambdaSymbol = OBJ_VAL(newSymbol("lambda", 6));
  push(lambdaSymbol);

  // Consume the 'lambda'.
  parserAdvance();

  Value argList = NIL_VAL;

  if (parserMatch(TOKEN_LEFT_PAREN)) {
    while (!check(TOKEN_RIGHT_PAREN)) {
      if (!check(TOKEN_IDENTIFIER)) {
        errorAtCurrent("Expect identifier in argument list of lambda.");
      }

      Value sym = symbol();
      push(sym);

      if (!IS_PAIR(argList)) {
        argList = CONS(sym, NIL_VAL);
        pop();  // sym
        push(argList);
      } else {
        append(AS_PAIR(argList), sym);
        pop();  // sym
      }
    }
  } else {
    consume(TOKEN_IDENTIFIER,
            "Expect argument list or argument variable after 'lambda'.");
    argList =
        OBJ_VAL(newSymbol(parser.previous->start, parser.previous->length));
    push(argList);
  }

  consume(TOKEN_RIGHT_PAREN, "Expect ')' to finish lambda argument list.");

  Value body = NIL_VAL;
  if (parserMatch(TOKEN_LEFT_PAREN)) {
    body = parseList(false);
  } else {
    body = parseExpression();
  }

  push(body);

  Value lambdaAndArgs = CONS(lambdaSymbol, argList);
  push(lambdaAndArgs);

  Value lambdaArgsBody = CONS(lambdaAndArgs, body);
  push(lambdaArgsBody);

  Value lambdaExpression = CONS(lambdaArgsBody, NIL_VAL);

  pop();  // lambdaArgsBody
  pop();  // lambdaAndArgs
  pop();  // body
  pop();  // argList
  pop();  // lambdaSymbol

  return lambdaExpression;
}

static bool atConditional() {
  return memcmp(parser.current->start, "if", min(2, parser.current->length)) ==
         0;
}

static Value conditional() {
  Value ifSymbol = OBJ_VAL(newSymbol("if", 2));
  push(ifSymbol);

  Value predicate = parseExpression();
  push(predicate);

  Value consequent = parseExpression();
  push(consequent);

  Value ifAndPred = CONS(ifSymbol, predicate);
  push(ifAndPred);

  Value ifPredConsq = CONS(ifAndPred, consequent);
  push(ifPredConsq);

  Value ifList = NIL_VAL;
  if (parserMatch(TOKEN_RIGHT_PAREN)) {
    ifList = CONS(ifPredConsq, NIL_VAL);
    pop();  // ifPredConsq
    pop();  // ifAndPred
    pop();  // consequent
    pop();  // predicate
    pop();  // ifSymbol
    return ifList;
  }

  Value alternate = parseExpression();
  push(alternate);

  Value ifPredConsqAlt = CONS(ifPredConsq, alternate);
  push(ifPredConsqAlt);

  ifList = CONS(ifPredConsqAlt, NIL_VAL);

  pop();  // ifPredConsqAlt
  pop();  // alternate
  pop();  // ifPredConsq
  pop();  // ifAndPred
  pop();  // consequent
  pop();  // predicate
  pop();  // ifSymbol

  return ifList;
}

static bool atAssignment() {
  return memcmp(parser.current->start, "set!",
                min(4, parser.current->length)) == 0;
}

static Value assignment() {
  Value setSymbol = OBJ_VAL(newSymbol("set!", 4));
  push(setSymbol);

  // consume 'set!'
  parserAdvance();

  consume(TOKEN_IDENTIFIER, "Expect identifier after 'set!'");

  Value target =
      OBJ_VAL(newSymbol(parser.previous->start, parser.previous->length));
  push(target);

  Value value = parseExpression();
  push(value);

  Value setAndTarget = CONS(setSymbol, target);
  push(setAndTarget);

  Value setTargetValue = CONS(setAndTarget, value);
  push(setTargetValue);

  Value setExpression = CONS(setTargetValue, NIL_VAL);

  pop();  // setTargetValue
  pop();  // setAndTarget
  pop();  // value
  pop();  // target
  pop();  // setSymbol

  return setExpression;
}

static Value listBasedExpression() {
  // Read the left paren
  parserAdvance();

  if (atLambda()) return lambda_();
  if (atConditional()) return conditional();
  if (atAssignment()) return assignment();

  DerivedExpressionType derivedType = NOT_DERIVED_EXPR;

  if ((derivedType = derivedExpressionType()) != NOT_DERIVED_EXPR) {
    return derivedExpression(derivedType);
  }

  // Procedure call
  return parseList(false);
}

Value parseExpression() {
  Value value = NIL_VAL;

  switch (parser.current->type) {
    // Literals
    // Self evaluating values
    case TOKEN_BOOLEAN:
      value = BOOL_VAL(booleanTokenToBool(parser.current));
      parserAdvance();
      break;
    case TOKEN_NUMBER:
      value = NUMBER_VAL(numberTokenToDouble(parser.current));
      parserAdvance();
      break;
    case TOKEN_CHARACTER:
      value = CHARACTER_VAL(characterTokenToChar(parser.current));
      break;
    case TOKEN_STRING:
      value =
          OBJ_VAL(copyString(parser.current->start, parser.current->length));
      break;
    case TOKEN_POUND_LEFT_PAREN:
      value = vector(false);
      break;
    case TOKEN_POUND_U8_LEFT_PAREN:
      value = bytevector();
      break;

      // Quotation
    case TOKEN_QUOTE:
      // Read the quote
      parserAdvance();
      value = parseDatum();
      break;

      // Identifiers
    case TOKEN_IDENTIFIER:
      value = symbol();
      break;

    case TOKEN_LEFT_PAREN:
      value = listBasedExpression();
      break;

    default:
      fprintf(stderr, "TODO: parse %s tokens.\n",
              tokenTypeToString(parser.current->type));
  }

  return value;
}
