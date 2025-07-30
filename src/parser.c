#include "parser.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "parser_internals/derived_expressions.h"
#include "parser_internals/literals.h"
#include "parser_internals/parser_operations.h"
#include "parser_internals/token_to_type.h"
#include "scanner.h"
#include "value.h"
#include "vm.h"

Parser parser;

static Value listBasedExpression();
static bool atLambda();
static Value lambda_();
static Value argumentList();
static bool atConditional();
static Value conditional();
static bool atAssignment();
static Value assignment();
static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }

void initParser(TokenArray tokens) {
  parser.tokens = tokens;
  parser.previous = tokens.array;
  parser.current = tokens.array;
  parser.hadError = false;
  parser.panicMode = false;
  parser.ast = NIL_VAL;
}

void markParserRoots() { markValue(parser.ast); }

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
      value = OBJ_VAL(tokenToObjString(parser.current));
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
      value = OBJ_VAL(tokenToObjString(parser.current));
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
      value = parseListOfDatums();
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
  return parseListOfExpressions();
}

static bool atLambda() {
  return memcmp(parser.current->start, "lambda",
                min(6, parser.current->length)) == 0;
}

static Value lambda_() {
  Value lambdaSymbol = OBJ_VAL(newSymbol("lambda", 6));
  push(lambdaSymbol);

  Value lambdaExpression = CONS(lambdaSymbol, NIL_VAL);
  pop();  // lambdaSymbol
  push(lambdaExpression);

  // Consume the 'lambda'.
  parserAdvance();

  Value argList = argumentList();
  push(argList);

  Value argListWrappedInPair = CONS(argList, NIL_VAL);
  pop();  // argList
  push(argListWrappedInPair);

  append(AS_PAIR(lambdaExpression), argListWrappedInPair);
  pop();  // argListWrappedInPair

  Value body = parseListOfExpressions();
  push(body);

  Value bodyWrappedInPair = CONS(body, NIL_VAL);
  pop();  // body
  push(bodyWrappedInPair);

  append(AS_PAIR(lambdaExpression), bodyWrappedInPair);
  pop();  // bodyWrappedInPair

  pop();  // lambdaExpression

  return lambdaExpression;
}

static Value argumentList() {
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
    argList = OBJ_VAL(tokenToObjString(parser.previous));
    push(argList);
  }

  pop();  // argList

  consume(TOKEN_RIGHT_PAREN, "Expect ')' to finish argument list.");
  return argList;
}

static bool atConditional() {
  return memcmp(parser.current->start, "if", min(2, parser.current->length)) ==
         0;
}

static Value conditional() {
  Value ifSymbol = OBJ_VAL(newSymbol("if", 2));
  push(ifSymbol);

  parserAdvance();  // Consume the 'if'

  Value ifExpression = CONS(ifSymbol, NIL_VAL);
  pop();  // ifSymbol
  push(ifExpression);

  Value predicate = parseExpression();
  push(predicate);

  Value predicateInPair = CONS(predicate, NIL_VAL);
  pop();  // predicate
  push(predicateInPair);

  append(AS_PAIR(ifExpression), predicateInPair);
  pop();  // predicateInPair

  Value consequent = parseExpression();
  push(consequent);

  Value consequentInPair = CONS(consequent, NIL_VAL);
  pop();  // consequent
  push(consequentInPair);

  append(AS_PAIR(ifExpression), NIL_VAL);
  pop();  // consequentInPair

  // Nothing more to do if there is no alternative
  if (parserMatch(TOKEN_RIGHT_PAREN)) {
    return ifExpression;
  }

  Value alternate = parseExpression();
  push(alternate);

  Value alternateInPair = CONS(alternate, NIL_VAL);
  pop();  // alternate
  push(alternateInPair);

  append(AS_PAIR(ifExpression), alternateInPair);
  pop();  // alternateInPair
  pop();  // ifExpression

  return ifExpression;
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

  Value target = OBJ_VAL(tokenToObjSymbol(parser.previous));
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
