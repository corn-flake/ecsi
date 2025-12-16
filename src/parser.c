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
#include "smart_array.h"
#include "value.h"
#include "vm.h"

Parser parser;

static void synchronize(void);
static Expr *parseListBasedExpression(void);
static ExprLiteral *parseQuotation(void);

static inline void initAST(AST *ast);
static void appendToAST(AST *ast, Expr *expr);

static void freeExpr(Expr *expr);

// Parses a procedure call.
static ExprCall *parseCall(void);

// Parses a lambda expression.
static ExprLambda *parseLambda(void);

// Parses an if expression.
static ExprIf *parseIf(void);

// Parses a set! expression.
static ExprSet *parseSet(void);

// Parses an identifier.
static ExprIdentifier *parseIdentifier(void);

// Parses a body (see r7rs standard, pg. 63)
static ExprBody *parseBody(void);

// Attempts to parse a definition. If it cannot parse one, it returns NULL.
static ExprDefinition *tryToParseDefinition(void);

// Initializes an ArgumentList.
static inline void initArgumentList(ArgumentList *argList);

// Parses as many identifiers as possible into an argument list.
static void parseArgumentList(ArgumentList *argList);

static inline void initExprPointerArray(ExprPointerArray *array);
static void appendToExprPointerArray(ExprPointerArray *array, Expr *expr);

void initParser(void) {
    parser.current = scanToken();
    parser.previous = parser.current;
    parser.hadError = false;
    parser.panicMode = false;
    initAST(&(parser.ast));
}

static void initAST(AST *ast) { initExprPointerArray(ast); }

void printAST(AST const *ast) { puts("TODO: printAST"); }

AST parseAllTokens(void) {
    while (!check(TOKEN_EOF)) {
        appendToAST(&(parser.ast), parseExpression());
    }
    return parser.ast;
}

void freeAST(AST *ast) {
    for (size_t i = 0; i < getSmartArrayCount(ast); i++) {
        freeExpr(SMART_ARRAY_AT(ast, i, Expr *));
    }
}

static void freeExpr(Expr *expr) { return; }

Expr *parseExpression(void) {
    switch (CURRENT_TYPE()) {
            // Literals
            // Self evaluating values
        case TOKEN_BOOLEAN:
            return (Expr *)parseBooleanNoCheck();
        case TOKEN_NUMBER:
            return (Expr *)parseNumberNoCheck();
        case TOKEN_CHARACTER:
            return (Expr *)parseCharacterNoCheck();
        case TOKEN_STRING:
            return (Expr *)parseStringNoCheck();
        case TOKEN_POUND_LEFT_PAREN:
            parserAdvance();
            return (Expr *)parseVector();
        case TOKEN_POUND_U8_LEFT_PAREN:
            parserAdvance();
            return (Expr *)parseBytevector();

            // Quotation
        case TOKEN_QUOTE:
            // Read the quote
            parserAdvance();
            return (Expr *)parseQuotation();

            // Identifiers
        case TOKEN_IDENTIFIER:
            return (Expr *)ALLOCATE_EXPR(ExprIdentifier, EXPR_IDENTIFIER,
                                         CURRENT_LOCATION());

        case TOKEN_LEFT_PAREN:
            parserAdvance();
            return parseListBasedExpression();

        case TOKEN_RIGHT_PAREN:
            errorAtCurrent("Unexpected right parenthesis.");
            parserAdvance();
            break;

        default:
            fprintf(stderr, "TODO: parse %s tokens.\n",
                    tokenTypeToString(tokenGetType(&(parser.current))));
    }

    if (parser.panicMode) synchronize();
    return NULL;
}

static Expr *parseListBasedExpression(void) {
    switch (CURRENT_TYPE()) {
        case TOKEN_QUOTE:
            parserAdvance();
            return (Expr *)parseQuotation();
        case TOKEN_IF:
            parserAdvance();
            return (Expr *)parseIf();
        case TOKEN_SET:
            parserAdvance();
            return (Expr *)parseSet();
        case TOKEN_LAMBDA:
            parserAdvance();
            return (Expr *)parseLambda();
        case TOKEN_AND:
        case TOKEN_OR:
            parserAdvance();
            return (Expr *)parseLogical();
        case TOKEN_WHEN:
        case TOKEN_UNLESS:
            parserAdvance();
            return (Expr *)parseWhenUnless();
        case TOKEN_BEGIN:
            parserAdvance();
            return (Expr *)parseBegin();
        default:
            return (Expr *)parseCall();
    }
}

static ExprIf *parseIf(void) {
    ExprIf *conditional = ALLOCATE_EXPR(ExprIf, EXPR_IF, CURRENT_LOCATION());
    conditional->test = parseExpression();
    conditional->consequent = parseExpression();

    conditional->alternate = NULL;

    if (!parserMatch(TOKEN_RIGHT_PAREN)) {
        conditional->alternate = parseExpression();
        consume(TOKEN_RIGHT_PAREN, "Expect ')' to close 'if' expression");
    }

    return conditional;
}

static ExprSet *parseSet(void) {
    ExprSet *assignment = ALLOCATE_EXPR(ExprSet, EXPR_SET, CURRENT_LOCATION());
    assignment->target = parseIdentifier();
    assignment->expression = parseExpression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close 'set!' expression");
    return assignment;
}

static ExprLambda *parseLambda(void) {
    ExprLambda *lambda =
        ALLOCATE_EXPR(ExprLambda, EXPR_LAMBDA, CURRENT_LOCATION());

    initArgumentList(&(lambda->formals));
    parseArgumentList(&(lambda->formals));

    lambda->body = parseBody();

    return lambda;
}

static void initArgumentList(ArgumentList *argList) {
    argList->type = ARG_LIST_NORMAL;
    initSmartArray(&(argList->identifiers), smartArrayCheckedRealloc,
                   sizeof(ExprIdentifier *));
}

static void parseArgumentList(ArgumentList *argList) {
    ExprIdentifier *id = NULL;
    argList->type = ARG_LIST_NORMAL;

    if (parserMatch(TOKEN_LEFT_PAREN)) {
        while (!check(TOKEN_RIGHT_PAREN)) {
            if (parserMatch(TOKEN_PERIOD)) {
                argList->type = ARG_LIST_VARIADIC;
            }
            id = parseIdentifier();
            smartArrayAppend(&(argList->identifiers), &id);
        }
    } else {
        argList->type = ARG_LIST_ONE_IDENTIFIER;
        id = parseIdentifier();
        smartArrayAppend(&(argList->identifiers), &id);
    }
}

static ExprBody *parseBody(void) {
    ExprBody *body = ALLOCATE_EXPR(ExprBody, EXPR_BODY, CURRENT_LOCATION());

    initExprPointerArray(&(body->definitions));

    for (ExprDefinition *definition = tryToParseDefinition();
         NULL != definition; definition = tryToParseDefinition()) {
        appendToExprPointerArray(&(body->definitions), (Expr *)definition);
    }

    body->sequence = parseExpressionsUntilRightParen();
}

static inline void initExprPointerArray(ExprPointerArray *array) {
    initSmartArray(array, smartArrayCheckedRealloc, sizeof(Expr *));
}

static void appendToExprPointerArray(ExprPointerArray *array, Expr *expr) {
    smartArrayAppend(array, &expr);
}

static ExprIdentifier *parseIdentifier(void) {
    if (!check(TOKEN_IDENTIFIER)) {
        errorAtCurrent("Expect identifier");
    }
    return ALLOCATE_EXPR(ExprIdentifier, EXPR_IDENTIFIER, CURRENT_LOCATION());
}

static ExprCall *parseCall(void) {
    ExprCall *call = ALLOCATE_EXPR(ExprCall, EXPR_CALL, CURRENT_LOCATION());
    call->operator= parseExpression();
    call->operands = parseUntilRightParen();
    return call;
}

static ExprLiteral *parseQuotation(void) {
    return makeLiteral(true, parseDatum());
}

static void appendToAST(AST *ast, Expr *expr) { smartArrayAppend(ast, &expr); }

static void synchronize(void) {
    parser.panicMode = false;

    while (!check(TOKEN_EOF)) {
        if (TOKEN_RIGHT_PAREN == tokenGetType(&(parser.previous))) return;
        if (check(TOKEN_LEFT_PAREN)) return;
        parserAdvance();
    }
}
