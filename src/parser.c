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

static SyntaxObject *parseList(void);
static SyntaxObject *parseAbbreviation(void);
static ObjSymbol *abbreviationSymbol(TokenType abbreviationPrefix);

static inline void initAST(SyntaxObjectArray *ast);
static void appendToAST(SyntaxObjectArray *ast, SyntaxObject *expr);

void initParser(void) {
    parser.current = scanToken();
    parser.previous = parser.current;
    parser.hadError = false;
    parser.panicMode = false;
    initAST(&(parser.ast));
    turnOffGarbageCollector();
}

static void initAST(SyntaxObjectArray *ast) {
    initSmartArray(ast, smartArrayCheckedRealloc, sizeof(SyntaxObject *));
}

void printAST(SyntaxObjectArray const *ast) {
    for (size_t i = 0, currentLine = 1; i < getSmartArrayCount(ast); i++) {
        if (SMART_ARRAY_AT(ast, i, SyntaxObject *)->location.line !=
            currentLine) {
            putchar('\n');
            currentLine = SMART_ARRAY_AT(ast, i, SyntaxObject *)->location.line;
        }

        printValue(SMART_ARRAY_AT(ast, i, SyntaxObject *)->value);
    }
}

SyntaxObjectArray parseAllTokens(void) {
    while (!check(TOKEN_EOF)) {
        appendToAST(&(parser.ast), parseExpression());
    }

    return parser.ast;
}

void freeAST(SyntaxObjectArray *ast) {
    freeSmartArray(ast);
    turnOnGarbageCollector();
    collectGarbage();
}

SyntaxObject *parseExpression(void) {
    if (tokenIsKeyword(&(parser.current)) || check(TOKEN_IDENTIFIER)) {
        ObjString *string = tokenToObjString(&(parser.current));
        SyntaxObject *identifier = makeSyntaxAtCurrent(OBJ_VAL(string));
        parserAdvance();
        return identifier;
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
        case TOKEN_COMMA:
        case TOKEN_COMMA_AT:
        case TOKEN_QUOTE:
            return parseAbbreviation();

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

static SyntaxObject *parseList(void) {
    consume(TOKEN_LEFT_PAREN, "Expect '(' to open list.");
    Token const *listStart = &(parser.previous);

    if (!canContinueList()) {
        if (check(TOKEN_RIGHT_PAREN)) {
            return makeSyntaxFromTokenToCurrent(NIL_VAL, listStart);
        } else {
            errorAtCurrent("Expect parenthesis to close list.");
        }
    }

    ObjPair *list = newPair(OBJ_VAL(parseExpression()), NIL_VAL);
    while (canContinueList()) {
        appendElement(list, OBJ_VAL(parseExpression()));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect right parenthesis to close list.");

    return makeSyntaxFromTokenToCurrent(OBJ_VAL(list), listStart);
}

static SyntaxObject *parseAbbreviation(void) {
    Value abbreviation =
        OBJ_VAL(newPair(OBJ_VAL(abbreviationSymbol(CURRENT_TYPE())), NIL_VAL));
    Token const *abbreviationStart = &(parser.current);
    Value datum = OBJ_VAL(parseExpression());
    SET_CDR(abbreviation, datum);
    return makeSyntaxFromTokenToCurrent(abbreviation, abbreviationStart);
}

static ObjSymbol *abbreviationSymbol(TokenType abbreviationPrefix) {
    switch (abbreviationPrefix) {
        case TOKEN_QUOTE:
            return newSymbol("quote", 5);
        case TOKEN_COMMA:
            return newSymbol("unquote", 7);
        case TOKEN_COMMA_AT:
            return newSymbol("unquote-splicing", 16);
        case TOKEN_BACKQUOTE:
            return newSymbol("quasiquote", 10);
        default:
            UNREACHABLE();
    }
}

static void appendToAST(SyntaxObjectArray *ast, SyntaxObject *expr) {
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
