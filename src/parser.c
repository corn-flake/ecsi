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
#include "value.h"
#include "vm.h"

Parser parser;

#define PLACEHOLDER_LOCATION \
    ((SourceLocation){.start = NULL, .length = 0, .line = CURRENT_LINE()})

static void synchronize(void);
static Expr *parseListBasedExpression(void);
static ExprLiteral *parseQuotation(void);
static void initAST(AST *ast);
static void appendToAST(AST *ast, Expr *expr);

static ExprCall *parseCall(void);
static ExprLambda *parseLambda(void);
static ExprIf *parseIf(void);
static ExprSet *parseSet(void);

void initParser(void) {
    parser.current = scanToken();
    parser.previous = parser.current;
    parser.hadError = false;
    parser.panicMode = false;
    initAST(&(parser.ast));
}

static void initAST(AST *ast) {
    initSmartArray(ast, smartArrayCheckedRealloc, sizeof(Expr *));
}

AST parseAllTokens(void) {
    while (!check(TOKEN_EOF)) {
        appendToAst(parseExpression());
    }
    return parser.ast;
}

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

static ExprCall *parseCall(void) {
    ExprCall *call = ALLOCATE_EXPR(ExprCall, EXPR_CALL, CURRENT_LOCATION());
    call->operator = parseExpression();
    call->operands = parseUntilRightParen();
    return (Expr *)call;
}

static ExprLiteral *parseQuotation(void) {
    return makeLiteral(true, parseDatum());
}

static void appendToAst(AST *ast, Expr *expr) {
    smartArrayAppend(ast, &expr);
}

static void synchronize(void) {
    parser.panicMode = false;

    while (!check(TOKEN_EOF)) {
        if (TOKEN_RIGHT_PAREN == tokenGetType(&(parser.previous))) return;
        if (check(TOKEN_LEFT_PAREN)) return;
        parserAdvance();
    }
}
