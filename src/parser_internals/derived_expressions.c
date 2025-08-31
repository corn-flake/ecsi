#include "derived_expressions.h"

#include "../object.h"
#include "../parser.h"
#include "../vm.h"
#include "literals.h"
#include "parser_operations.h"

/*
  NOTE: In this file, you will see the idiom:
  Value xyz = CONS(abc, NIL_VAL);
  push(xyz);
  SET_CDR(xyz, CONS(def, NIL_VAL));
  pop(); // xyz
  often. This would be nicely expressed as
  Value xyz = CONS(abc, CONS(def, NIL_VAL));
  however, note that the result of the inner CONS
  can only be found on the C stack. Therefore,
  we must seperate the calls and push xyz between them.
  We use SET_CDR instead of append because in this case,
  SET_CDR is more efficient.
 */

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

Value parseAnd() { return parseListOfExpressions(); }
