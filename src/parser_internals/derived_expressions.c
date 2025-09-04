#include "derived_expressions.h"

#include "../object.h"
#include "../parser.h"
#include "../vm.h"
#include "literals.h"
#include "parser_operations.h"

static Value parseBindingSpec();
static Value parseIf();
static Value parseAnd();
static Value parseOr();
static Value parseLet();

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

    push(condition);
    Value ifExpr = CONS(condition, NIL_VAL);
    pop();  // condition
    push(ifExpr);

    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        error("Expect expression as consequent in 'if' expression.");
    }

    Value consequent = parseExpression();
    push(consequent);
    append(AS_PAIR(ifExpr), consequent);
    pop();  // consequent

    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        // No alternative
        return ifExpr;
    }

    Value alternative = parseExpression();
    push(alternative);
    append(AS_PAIR(ifExpr), alternative);
    pop();  // alternative

    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close if statement.");

    return ifExpr;
}

static Value parseOr() { return parseListOfExpressions(); }

static Value parseLet() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'let'");

    Value bindings = parseListUsing(parseBindingSpec);
    push(bindings);

    Value exprs = parseListOfExpressions();
    push(exprs);

    Value letExpr = CONS(bindings, exprs);

    pop();  // exprs
    pop();  // bindings
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
    push(value);

    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close binding spec.");

    Value spec = CONS(target, NIL_VAL);
    push(spec);
    SET_CDR(spec, CONS(value, NIL_VAL));

    pop();  // spec
    pop();  // value
    pop();  // target

    return spec;
}

static Value parseAnd() { return parseListOfExpressions(); }
