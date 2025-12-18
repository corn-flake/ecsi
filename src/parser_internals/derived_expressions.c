/*
  Copyright 2025 Evan Cooney

  This file is part of Ecsi.

  Ecsi is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  Ecsi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  Ecsi. If not, see <https://www.gnu.org/licenses/>.
 */

#include "derived_expressions.h"

#include "parser_operations.h"

// static Value parseLet();
// static Value parseBindingSpec();
// static Value parseCond();
// static Value parseDefine();
// static Value parseWhen();
// static Value parseUnless();

/*
  Parses a cond clause, raising an error if *foundElse
  is true and it finds an else clause. It sets *foundElse
  to true if it finds an else clause.
*/
// static Value parseCondClause(bool *foundElse);

ParseDatumFn getDerivedExpressionParseFn(void) {
    /*
    if (!check(TOKEN_IDENTIFIER)) return NULL;
    if (currentTokenMatchesString("cond")) return parseCond;
    if (currentTokenMatchesString("let")) return parseLet;
    if (currentTokenMatchesString("and")) return parseAnd;
    if (currentTokenMatchesString("or")) return parseOr;
    if (currentTokenMatchesString("if")) return parseIf;
    if (currentTokenMatchesString("define")) return parseDefine;
    if (currentTokenMatchesString("when")) return parseWhen;
    if (currentTokenMatchesString("unless")) return parseUnless;
    if (currentTokenMatchesString("begin")) return parseBegin;
    if (currentTokenMatchesString("lambda")) return parseLambda;
    */
    return NULL;
}
/*
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
*/

/*
static Value parseLet(void) {
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
*/

/*
static Value parseCond() {
    bool foundElse = false;
    Value currentClause = parseCondClause(&foundElse);
    Value clauseList = guardedCons(currentClause, NIL_VAL);
*/
/*
 We don't return here if foundElse is true because that would lead to an
 error where duplicate else clauses are not detected if the first clause is
 an else clause.
*/
/*
    // We push these to protect them from parseCondClause.
    push(clauseList);
    push(currentClause);

    while (canContinueList()) {
        currentClause = parseCondClause(&foundElse);
        guardedAppend(clauseList, currentClause);
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close cond clause.");

    pop();  // currentClause
    pop();  // clauseList

    return clauseList;
}

static Value parseCondClause(bool *foundElse) {
    consume(TOKEN_LEFT_PAREN, "Expect '(' to start cond clause.");

    if (check(TOKEN_IDENTIFIER) && currentTokenMatchesString("else")) {
        if (*foundElse) {
            // Two else clauses
            error("You can only have one 'else' clause in a cond expression.");
        } else {
            *foundElse = true;
        }
    }

    Value condition = parseExpression();

    push(condition);
    Value consequent = parseListOfExpressions();
    pop();  // condition

    Value condClause = guardedCons(condition, consequent);
    return condClause;
}

static Value parseDefine() {
    Value firstPart = NIL_VAL;
    Value secondPart = NIL_VAL;

    if (parserMatch(TOKEN_LEFT_PAREN)) {
        // Function definition
        firstPart = parseArgList();
        push(firstPart);
        secondPart = parseListOfExpressions();
        if (IS_NIL(secondPart)) {
            error(
                "Expect at least one expression in body of define expression.");
        }
        pop();  // firstPart
    } else {
        // Variable definition
        firstPart = symbol();
        push(firstPart);
        secondPart = parseExpression();
        pop();  // firstPart
        consume(TOKEN_RIGHT_PAREN, "Expect ')' to close define expression.");
    }

    push(secondPart);
    Value defineExpression = guardedCons(firstPart, secondPart);
    pop();  // secondPart
    return defineExpression;
}
*/

/*
static Value parseWhen() {
    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        error("Expect expression as test in unless expression.");
        return NIL_VAL;
    }

    Value test = parseExpression();
    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        error(
            "Expect at least one expression as consequent of unless "
            "expression.");
    }

    push(test);
    Value firstExpr = parseExpression();
    push(firstExpr);

    Value restExprs = parseListOfExpressions();
    pop();  // firstExpr

    Value sequence = guardedCons(firstExpr, restExprs);
    pop();  // test
    Value whenExpr = guardedCons(test, sequence);
    return whenExpr;
}


static Value parseUnless() {
    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        error("Expect expression as test in unless expression.");
        return NIL_VAL;
    }

    Value test = parseExpression();
    push(test);
    Value sequence = parseListOfExpressions();
    pop();  // test
    Value unlessExpr = guardedCons(test, sequence);
    return unlessExpr;
}

static Value parseBegin() {
    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        error("Expect at least one expression in begin expression.");
    }
    return parseListOfExpressions();
}
*/

/*
static Value parseLambda() {
    Value args = NIL_VAL;

    if (parserMatch(TOKEN_LEFT_PAREN)) {
        args = parseArgList();
    } else {
        args = symbol();
    }

    push(args);
    Value body = parseListOfExpressions();
    if (IS_NIL(body)) {
        error("Expect at least one expression in body of lambda expression.");
    }
    pop();  // args
    return guardedCons(args, body);
}
*/

/*
static Value parseArgList() {
    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        return NIL_VAL;
    }

    Value arg = symbol();
    Value argList = guardedCons(arg, NIL_VAL);
    Value lastPair = argList;

    push(argList);
    while (canContinueList() && !check(TOKEN_PERIOD)) {
        arg = symbol();
        guardedAppend(argList, arg);
        lastPair = CDR(argList);
    }
    pop();  // argList

    if (parserIsAtEnd()) {
        errorAtCurrent("Expected ')' to close argument list.");
    }

    if (parserMatch(TOKEN_RIGHT_PAREN)) {
        return argList;
    }

    // Variadic arguments

    // Consume the period
    parserAdvance();

    push(argList);
    arg = symbol();  // argument that holds variadic args
    pop();           // argList
    SET_CDR(lastPair, arg);
    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close argument list.");
    return argList;
}
*/
