#include "derived_expressions.h"

#include "../object.h"
#include "../parser.h"
#include "../vm.h"
#include "literals.h"
#include "parser_operations.h"

static Value condClause();

ParseFn getDerivedExpressionParseFn() {
    if (currentTokenMatchesString("cond")) {
        return cond;
    }

    if (currentTokenMatchesString("let")) {
        return let;
    }

    if (currentTokenMatchesString("and")) {
        return and_;
    }

    if (currentTokenMatchesString("or")) {
        return or_;
    }

    return NULL;
}

Value cond() {
    Value condSymbol = OBJ_VAL(newSymbol("cond", 4));
    push(condSymbol);

    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'cond'.");

    if (currentTokenMatchesString("else")) {
        Value elseSymbol = symbol();
        push(elseSymbol);

        Value sequence = parseListOfExpressions();
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

static Value condClause() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' to start cond clause.");
    Value test = parseExpression();
    push(test);

    Value sequence = parseListOfExpressions();
    push(sequence);

    Value testAndSequence = CONS(test, sequence);
    push(testAndSequence);

    Value condClause = CONS(testAndSequence, NIL_VAL);

    pop();  // testAndSequence
    pop();  // sequence
    pop();  // test

    return condClause;
}

Value or_() {
    Value orSymbol = OBJ_VAL(newSymbol("or", 2));
    push(orSymbol);

    // consume 'or'
    parserAdvance();

    Value expressions = parseListOfExpressions();
    push(expressions);

    Value orExpression = CONS(orSymbol, expressions);

    pop();  // expressions
    pop();  // orSymbol

    return orExpression;
}

static Value bindingSpec() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' to start binding spec.");
    consume(TOKEN_IDENTIFIER, "Expect identifier after '(' in binding spec.");

    Value target = OBJ_VAL(tokenToObjSymbol(parser.previous));
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

    Value body = parseListOfExpressions();
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

    Value expressions = parseListOfExpressions();
    push(expressions);

    Value andExpression = CONS(andSymbol, expressions);

    pop();  // expressions
    pop();  // andSymbol

    return andExpression;
}
