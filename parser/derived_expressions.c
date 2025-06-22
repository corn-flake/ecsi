#include "derived_expressions.h"

#include "../object.h"
#include "../vm.h"
#include "literals.h"
#include "parser.h"

static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }

DerivedExpressionType derivedExpressionType() {
  if (!memcmp(parser.current->start, "cond", min(4, parser.current->length))) {
    return DERIVED_EXPR_COND;
  }

  if (!memcmp(parser.current->start, "let", min(3, parser.current->length))) {
    return DERIVED_EXPR_LET;
  }

  if (!memcmp(parser.current->start, "and", min(3, parser.current->length))) {
    return DERIVED_EXPR_AND;
  }

  if (!memcmp(parser.current->start, "or", min(2, parser.current->length))) {
    return DERIVED_EXPR_OR;
  }

  return NOT_DERIVED_EXPR;
}

Value derivedExpression(DerivedExpressionType type) {
  switch (type) {
    case DERIVED_EXPR_AND:
      return and_();
    case DERIVED_EXPR_COND:
      return cond();
    case DERIVED_EXPR_LET:
      return let();
    case DERIVED_EXPR_OR:
      return or_();
    default:
      return NIL_VAL;
  }
}

static Value condClause() {
  consume(TOKEN_LEFT_PAREN, "Expect '(' to start cond clause.");
  Value test = parseExpression();
  push(test);

  Value sequence = parseList(false);
  push(sequence);

  Value testAndSequence = CONS(test, sequence);
  push(testAndSequence);

  Value condClause = CONS(testAndSequence, NIL_VAL);

  pop();  // testAndSequence
  pop();  // sequence
  pop();  // test

  return condClause;
}

Value cond() {
  Value condSymbol = OBJ_VAL(newSymbol("cond", 4));
  push(condSymbol);

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'cond'.");

  if (memcmp(parser.current->start, "else", min(4, parser.current->length)) ==
      0) {
    Value elseSymbol = symbol();
    push(elseSymbol);

    Value sequence = parseList(false);
    push(sequence);

    Value elseAndSequence = CONS(elseSymbol, sequence);
    push(elseAndSequence);

    Value elseClause = CONS(elseAndSequence, NIL_VAL);
    push(elseClause);

    Value condAndElse = CONS(condSymbol, elseClause);
    push(condAndElse);

    Value condExpression = CONS(condAndElse, NIL_VAL);

    pop();  // condAndElse
    pop();  // elseClause
    pop();  // elseAndSequence
    pop();  // sequence
    pop();  // elseSymbol
    pop();  // condSymbol

    return condExpression;
  }

  Value clause1 = condClause();
  push(clause1);
  Value clauses = CONS(clause1, NIL_VAL);
  pop();  // clause1

  push(clauses);

  while (!check(TOKEN_RIGHT_PAREN)) {
    Value clause = condClause();
    push(clause);
    append(AS_PAIR(clauses), clause);
    pop();  // clause
  }

  Value condAndClauses = CONS(condSymbol, clauses);
  push(condAndClauses);

  Value condExpression = CONS(condAndClauses, NIL_VAL);

  pop();  // condAndClauses
  pop();  // clauses
  pop();  // condSymbol

  return condExpression;
}

Value or_() {
  Value orSymbol = OBJ_VAL(newSymbol("or", 2));
  push(orSymbol);

  // consume 'or'
  parserAdvance();

  Value expressions = parseList(false);
  push(expressions);

  Value orExpression = CONS(orSymbol, expressions);

  pop();  // expressions
  pop();  // orSymbol

  return orExpression;
}

static Value bindingSpec() {
  consume(TOKEN_LEFT_PAREN, "Expect '(' to start binding spec.");
  consume(TOKEN_IDENTIFIER, "Expect identifier after '(' in binding spec.");

  Value target =
      OBJ_VAL(newSymbol(parser.previous->start, parser.previous->length));
  push(target);

  Value value = parseExpression();
  push(value);

  Value targetAndValue = CONS(target, value);
  push(targetAndValue);

  Value spec = CONS(targetAndValue, NIL_VAL);

  pop();  // targetAndValue
  pop();  // value
  pop();  // target

  return spec;
}

Value let() {
  Value letSymbol = OBJ_VAL(newSymbol("let", 3));
  push(letSymbol);

  // consume 'let'
  parserAdvance();

  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'let'.");

  Value bindings = CONS(bindingSpec(), NIL_VAL);
  push(bindings);
  int bindingsCount = 1;

  while (parser.current->type != TOKEN_RIGHT_PAREN) {
    bindings = CONS(bindingSpec(), bindings);
    push(bindings);
    bindingsCount++;
  }

  Value body = parseList(false);
  push(body);

  Value letAndBindings = CONS(letSymbol, bindings);
  push(letAndBindings);

  Value letBindingsBody = CONS(letAndBindings, body);
  push(letBindingsBody);

  Value letExpression = CONS(letBindingsBody, NIL_VAL);

  pop();  // letBindingsBody
  pop();  // letAndBindings
  pop();  // body

  // bindings
  while (bindingsCount--) {
    pop();
  }

  pop();  // letSymbol

  return letExpression;
}

Value and_() {
  Value andSymbol = OBJ_VAL(newSymbol("and", 3));
  push(andSymbol);

  // consume 'and'
  parserAdvance();

  Value expressions = parseList(false);
  push(expressions);

  Value andExpression = CONS(andSymbol, expressions);

  pop();  // expressions
  pop();  // andSymbol

  return andExpression;
}
