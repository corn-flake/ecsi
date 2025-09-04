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

ParseFn getDerivedExpressionParseFn() {
    if (!check(TOKEN_IDENTIFIER)) {
        return NULL;
    }

    /*
    if (currentTokenMatchesString("cond")) {
        return cond;
    }
    */

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
