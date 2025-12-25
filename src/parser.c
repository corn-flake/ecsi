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

#include "common.h"
#include "memory.h"
#include "object.h"
#include "parser_internals/literals.h"
#include "parser_internals/parser_operations.h"
#include "scanner.h"
#include "smart_array.h"
#include "value.h"

Parser parser;

static void synchronize(void);

static ObjSyntax *parseList(void);
static ObjSyntax *parseAbbreviation(ObjSymbol *abbreviationPrefix);

static inline void initAST(ObjSyntaxPointerArray *ast);
static void appendToAST(ObjSyntaxPointerArray *ast, ObjSyntax *expr);

void initParser(void) {
    parser.current = scanToken();
    parser.previous = parser.current;
    parser.hadError = false;
    parser.panicMode = false;
    initAST(&(parser.ast));
    turnOffGarbageCollector();
}

static void initAST(ObjSyntaxPointerArray *ast) {
    initSmartArray(ast, smartArrayCheckedRealloc, sizeof(ObjSyntax *));
}

void printAST(ObjSyntaxPointerArray const *ast) {
    for (size_t i = 0, currentLine = 1; i < getSmartArrayCount(ast); i++) {
        if (SMART_ARRAY_AT(ast, i, ObjSyntax *)->location.line != currentLine) {
            putchar('\n');
            currentLine = SMART_ARRAY_AT(ast, i, ObjSyntax *)->location.line;
        }

        printValue(SMART_ARRAY_AT(ast, i, ObjSyntax *)->value);
    }
}

ObjSyntaxPointerArray parseAllTokens(void) {
    while (!parserIsAtEnd()) {
        appendToAST(&(parser.ast), parseExpression());
    }

    return parser.ast;
}

void freeAST(ObjSyntaxPointerArray *ast) {
    freeSmartArray(ast);
    turnOnGarbageCollector();
    collectGarbage();
}

ObjSyntax *parseExpression(void) {
    if (tokenIsKeyword(&(parser.current)) || check(TOKEN_IDENTIFIER)) {
        return parseSymbol();
    }

    switch (CURRENT_TYPE()) {
        // Simple datums
        case TOKEN_BOOLEAN:
            return parseBoolean();
        case TOKEN_NUMBER:
            return parseNumber();
        case TOKEN_CHARACTER:
            return parseCharacter();
        case TOKEN_STRING:
            return parseString();
        case TOKEN_POUND_U8_LEFT_PAREN:
            return parseBytevector();

        // Compound datums
        case TOKEN_LEFT_PAREN:
            return parseList();
        case TOKEN_POUND_LEFT_PAREN:
            return parseVector();
        case TOKEN_BACKQUOTE:
            return parseAbbreviation(newSymbol("quasiquote", 10));
        case TOKEN_COMMA:
            return parseAbbreviation(newSymbol("unquote", 7));
        case TOKEN_COMMA_AT:
            return parseAbbreviation(newSymbol("unquote-splicing", 16));
        case TOKEN_QUOTE:
            return parseAbbreviation(newSymbol("quote", 5));

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

static ObjSyntax *parseList(void) {
    consume(TOKEN_LEFT_PAREN, "Expect '(' to open list.");
    Token const listStart = parser.previous;

    if (!canContinueList()) {
        if (parserMatch(TOKEN_RIGHT_PAREN)) {
            return makeSyntaxFromTokenToCurrent(NIL_VAL, &listStart);
        } else {
            errorAtCurrent("Expect parenthesis to close list.");
        }
    }

    ObjSyntax *expr = parseExpression();
    ObjPair *list = newPair(OBJ_VAL(expr), NIL_VAL);
    while (canContinueList()) {
        expr = parseExpression();
        appendElement(list, OBJ_VAL(expr));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect right parenthesis to close list.");

    return makeSyntaxFromTokenToCurrent(OBJ_VAL(list), &listStart);
}

static ObjSyntax *parseAbbreviation(ObjSymbol *abbreviationPrefix) {
    ObjPair *abbreviation = newPair(OBJ_VAL(abbreviationPrefix), NIL_VAL);
    Token const abbreviationStart = parser.current;
    parserAdvance();
    Value datum = OBJ_VAL(parseExpression());
    appendElement(abbreviation, datum);
    return makeSyntaxFromTokenToCurrent(OBJ_VAL(abbreviation),
                                        &abbreviationStart);
}

static void appendToAST(ObjSyntaxPointerArray *ast, ObjSyntax *expr) {
    smartArrayAppend(ast, &expr);
}

static void synchronize(void) {
    parser.panicMode = false;

    while (!parserIsAtEnd()) {
        if (TOKEN_RIGHT_PAREN == tokenGetType(&(parser.previous))) return;
        if (check(TOKEN_LEFT_PAREN)) return;
        parserAdvance();
    }
}
