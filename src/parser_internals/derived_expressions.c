#include "derived_expressions.h"

#include "../object.h"
#include "../parser.h"
#include "../vm.h"
#include "literals.h"
#include "parser_operations.h"

static Value parseIf();
static Value parseAnd();
static Value parseOr();
static Value parseLet();
static Value parseBindingSpec();
static Value parseCond();

/*
  Parses a cond clause, raising an error if *foundElse
  is true and it finds an else clause. It sets *foundElse
  to true if it finds an else clause.
*/
static Value parseCondClause(bool *foundElse);

ParseFn getDerivedExpressionParseFn() {
    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    if (currentTokenMatchesString("cond")) {
        return parseCond;
    }

    if (currentTokenMatchesString("let")) {
        return parseLet;
    }

    if (currentTokenMatchesString("and")) {
        return parseAnd;
    }

    if (currentTokenMatchesString("or")) {
        return parseOr;
    }

    if (currentTokenMatchesString("if")) {
        return parseIf;
    }

    return NULL;
}

static Value parseIf() {
    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        error("Expect expression after 'if'.");
    }

    Value condition = parseExpression();

    Value ifExpr = guardedCons(condition, NIL_VAL);

    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        error("Expect expression as consequent in 'if' expression.");
    }

    push(ifExpr);
    Value consequent = parseExpression();
    pop();  // ifExpr

    guardedAppend(ifExpr, consequent);

    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        // No alternative
        return ifExpr;
    }

    push(ifExpr);
    Value alternative = parseExpression();
    pop();  // ifExpr

    guardedAppend(ifExpr, alternative);

    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close if statement.");

    return ifExpr;
}

static Value parseAnd() { return parseListOfExpressions(); }

static Value parseOr() { return parseListOfExpressions(); }

static Value parseLet() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'let'");

    Value bindings = parseListUsing(parseBindingSpec);

    push(bindings);
    Value exprs = parseListOfExpressions();
    pop();  // bindings

    Value letExpr = guardedCons(bindings, exprs);

    return letExpr;
}

static Value parseBindingSpec() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' to start binding spec.");
    if (!check(TOKEN_IDENTIFIER)) {
        errorAtCurrent("Expect identifier as first element in binding spec.");
    }

    Value target = symbol();

    push(target);
    Value value = parseExpression();
    pop();  // target

    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close binding spec.");

    push(value);
    Value spec = guardedCons(target, NIL_VAL);
    pop();  // value

    push(spec);
    SET_CDR(spec, guardedCons(value, NIL_VAL));
    pop();  // spec

    return spec;
}

static Value parseCond() {
    bool foundElse = false;
    Value currentClause = parseCondClause(&foundElse);
    Value clauseList = guardedCons(currentClause, NIL_VAL);
    /*
     We don't return here if foundElse is true because that would lead to an
     error where duplicate else clauses are not detected if the first clause is
     an else clause.
    */

    while (canContinueList()) {
        currentClause = parseCondClause(&foundElse);
        guardedAppend(clauseList, currentClause);
    }

    return clauseList;
}
static Value parseCondClause(bool *foundElse) {
    consume(TOKEN_LEFT_PAREN, "Expect '(' to start cond clause.");
    Value condition = parseExpression();

    bool isElseClause =
        IS_SYMBOL(CAR(condition)) &&
        textOfSymbolEqualToString(AS_SYMBOL(CAR(condition)), "else");

    if (*foundElse && isElseClause) {
        // Two else clauses
        error("You can only have one 'else' clause in a cond expression.");
    } else if (!(*foundElse) && isElseClause) {
        *foundElse = true;
    }

    push(condition);
    Value consequent = parseListOfExpressions();
    pop();  // condition

    Value condClause = guardedCons(condition, consequent);
    return condClause;
}
