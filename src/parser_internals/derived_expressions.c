#include "derived_expressions.h"

#include "../object.h"
#include "../parser.h"
#include "../vm.h"
#include "literals.h"
#include "parser_operations.h"

static Value parseBindingSpec();

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

    return NULL;
}

Value parseOr() { return parseListOfExpressions(); }

Value parseLet() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'let'");

    Value bindings = parseListUsing(parseBindingSpec);
    push(bindings);

    Value exprs = parseListOfExpressions();
    push(exprs);

    Value letExpr = CONS(bindings, CONS(exprs, NIL_VAL));
    pop();  // exprs
    pop();  // bindings
    return letExpr;
}

static Value parseBindingSpec() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' to start binding spec.");
    consume(TOKEN_IDENTIFIER, "Expect identifier after '(' in binding spec.");

    Value target = symbol();
    push(target);

    Value value = parseExpression();
    push(value);

    Value spec = CONS(target, CONS(value, NIL_VAL));

    pop();  // value
    pop();  // target

    return spec;
}

Value parseAnd() { return parseListOfExpressions(); }
