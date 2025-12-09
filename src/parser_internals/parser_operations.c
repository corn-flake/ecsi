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

#include "parser_operations.h"

#include <stdarg.h>
#include <stdio.h>

#include "../memory.h"
#include "../object.h"
#include "../parser.h"
#include "../scanner.h"
#include "../value.h"
#include "../vm.h"

static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }
static bool tokensEqual(Token const *t1, Token const *t2);

Expr *allocateExpr(size_t size, ExprType type, SourceLocation location) {
    // We cast to Expr * to make it clear that this is an Expr object
    // even though it might have a different size.
    Expr *expr = (Expr *)checkedMalloc(size);
    expr->type = type;
    expr->location = location;
    return expr;
}

void errorAt(Token const *token, char const *message) {
    formattedErrorAt(token, "%s", message);
}

void formattedErrorAt(Token const *token, char const *format, ...) {
    va_list args;
    va_start(args, format);
    varArgsFormattedErrorAt(token, format, args);
    va_end(args);
}

void error(char const *message) { errorAt(&(parser.previous), message); }

void formattedError(char const *format, ...) {
    va_list args;
    va_start(args, format);
    varArgsFormattedErrorAt(&(parser.previous), format, args);
    va_end(args);
}

void errorAtCurrent(char const *message) {
    errorAt(&(parser.current), message);
}

void formattedErrorAtCurrent(char const *format, ...) {
    va_list args;
    va_start(args, format);
    varArgsFormattedErrorAt(&(parser.current), format, args);
    va_end(args);
}

void varArgsFormattedErrorAt(Token const *token, char const *format,
                             va_list args) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %zu] Error", tokenGetLine(token));
    if (TOKEN_EOF == tokenGetType(token)) {
        fprintf(stderr, " at end");
    } else if (TOKEN_ERROR == tokenGetType(token)) {
        /*
          We don't do anything if the token is an error because if it is,
          the token's error message will be passed as the MESSAGE
         */
    } else {
        fprintf(stderr, " at '%.*s'", (int)tokenGetLength(token),
                tokenGetStart(token));
    }
    fprintf(stderr, ": ");
    vfprintf(stderr, format, args);
    fputs("\n", stderr);

    parser.hadError = true;
}

void parserAdvance(void) {
    parser.previous = parser.current;

    while (true) {
        parser.current = scanToken();
        if (TOKEN_ERROR == tokenGetType(&(parser.current))) break;
        errorAt(&(parser.current), tokenGetStart(&(parser.current)));
    }
}

void consume(TokenType type, char const *message) {
    if (type == tokenGetType(&(parser.current))) {
        parserAdvance();
        return;
    }
    errorAtCurrent(message);
}

bool check(TokenType type) { return type == tokenGetType(&(parser.current)); }

bool matchToken(Token const *token) {
    if (checkToken(token)) {
        parserAdvance();
        return true;
    }
    return false;
}

bool checkToken(Token const *token) {
    return tokensEqual(token, &(parser.current));
}

bool parserIsAtEnd(void) { return check(TOKEN_EOF); }

static bool tokensEqual(Token const *t1, Token const *t2) {
    return (tokenGetType(t1) == tokenGetType(t2)) &&
           (tokenGetLength(t1) == tokenGetLength(t2)) &&
           !memcmp(tokenGetStart(t1), tokenGetStart(t2), tokenGetLength(t1));
}

bool parserMatch(TokenType type) {
    if (!check(type)) return false;
    parserAdvance();
    return true;
}

bool canContinueList(void) {
    return !check(TOKEN_RIGHT_PAREN) && !check(TOKEN_EOF);
}

bool tokenMatchesString(Token *token, char *const string) {
    return !memcmp(tokenGetStart(token), string,
                   min(strlen(string), tokenGetLength(token)));
}

bool currentTokenMatchesString(char *const string) {
    return tokenMatchesString(&(parser.current), string);
}

bool previousTokenMatchesString(char *const string) {
    return tokenMatchesString(&(parser.previous), string);
}

Value parseListUsing(ParseDatumFn parse) {
    return parseNExprsIntoList(parse, -1);
}

Value parseNExprsIntoList(ParseDatumFn parse, int n) {
    if (0 == n) {
        consume(TOKEN_RIGHT_PAREN,
                "Expect right parenthesis to finish empty list");
        return NIL_VAL;
    }

    if (!canContinueList()) {
        if (parserMatch(TOKEN_RIGHT_PAREN)) {
            if (-1 != n) {
                formattedErrorAtCurrent(
                    "Expected list of %d elements but didn't find any "
                    "elements.",
                    n);
            }
            return NIL_VAL;
        }

        errorAtCurrent("Expect ')' to close list.");
        return NIL_VAL;
    }

    Value expr = parse();

    Value list = guardedCons(expr, NIL_VAL);
    // We push here to avoid pushing in every iteration before calling
    // parse.
    push(list);

    if (-1 == n) {
        while (canContinueList()) {
            expr = parse();
            guardedAppend(list, expr);
        }
    } else {
        for (int i = 1; i < n; i++) {
            if (check(TOKEN_RIGHT_PAREN)) {
                formattedErrorAtCurrent(
                    "Expected list of %d elements but only found %d "
                    "elements.",
                    n, i);
                break;
            }
            expr = parse();
            guardedAppend(list, expr);
        }
    }

    pop();  // list
    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close list.");

    return list;
}

Value parseAtLeastNExprsUsing(ParseDatumFn parse, size_t n) {
    if (0 == n && parserMatch(TOKEN_RIGHT_PAREN)) {
        return NIL_VAL;
    }

    // At this point we know there is at least one expression to parse,
    // so it is safe to call parse. Even if the user expects 0, it's fine.
    Value expr = parse();
    Value list = guardedCons(expr, NIL_VAL);
    size_t exprsParsed = 1;

    push(list);
    for (; exprsParsed < n && canContinueList(); exprsParsed++) {
        expr = parse();
        appendElement(AS_PAIR(list), expr);
    }
    pop();  // list

    // Not enough exprs
    if (exprsParsed < n) {
        formattedErrorAtCurrent(
            "Expected at least %d expressions but only found %d", n,
            exprsParsed);
        return list;
    }

    push(list);
    while (canContinueList()) {
        expr = parse();
        appendElement(AS_PAIR(list), expr);
    }
    pop();  // list

    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close list.");

    return list;
}

Value parseListOfExpressions() { return parseListUsing(parseExpression); }
