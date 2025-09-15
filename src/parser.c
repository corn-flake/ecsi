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

static void appendToAst(Value value);
static void synchronize();
static Value parseListBasedExpression();
static Value parseQuotation();

void initParser(TokenArray tokens) {
    parser.tokens = tokens;
    parser.previous = tokens.array;
    parser.current = tokens.array;
    parser.hadError = false;
    parser.panicMode = false;
    parser.ast = NIL_VAL;
}

void markParserRoots() { markValue(parser.ast); }

Value parseAllTokens() {
    while (!check(TOKEN_EOF)) {
        appendToAst(parseExpression());
    }
    return parser.ast;
}

Value parseExpression() {
    Value value = NIL_VAL;

    switch (parser.current->type) {
        // Literals
        // Self evaluating values
        case TOKEN_BOOLEAN:
            value = parseBooleanNoCheck();
            break;
        case TOKEN_NUMBER:
            value = parseNumberNoCheck();
            break;
        case TOKEN_CHARACTER:
            value = parseCharacterNoCheck();
            break;
        case TOKEN_STRING:
            value = parseStringNoCheck();
            break;
        case TOKEN_POUND_LEFT_PAREN:
            parserAdvance();
            value = parseVectorUsing(parseExpression);
            break;
        case TOKEN_POUND_U8_LEFT_PAREN:
            parserAdvance();
            value = parseBytevector();
            break;

            // Quotation
        case TOKEN_QUOTE:
            // Read the quote
            parserAdvance();
            value = parseQuotation();
            break;

            // Identifiers
        case TOKEN_IDENTIFIER:
            value = symbol();
            break;

        case TOKEN_LEFT_PAREN: {
            parserAdvance();
            value = parseListBasedExpression();
            break;
        }

        case TOKEN_RIGHT_PAREN:
            errorAtCurrent("Unexpected right parenthesis.");
            parserAdvance();
            break;

        default:
            fprintf(stderr, "TODO: parse %s tokens.\n",
                    tokenTypeToString(parser.current->type));
    }

    if (parser.panicMode) synchronize();
    return value;
}

static Value parseListBasedExpression() {
    ParseFn parse = getDerivedExpressionParseFn();

    // Derived expression
    if (NULL != parse) {
        Value expr = guardedCons(symbol(), NIL_VAL);

        push(expr);
        Value rest = parse();
        pop();  // expr

        SET_CDR(expr, rest);

        return expr;
    }

    // Procedure call
    return parseListOfExpressions();
}

static Value parseQuotation() {
    size_t const QUOTE_LEN = 5;
    Value quoteSymbol = OBJ_VAL(newSymbol("quote", QUOTE_LEN));
    Value list = guardedCons(quoteSymbol, NIL_VAL);

    push(list);
    Value expr = parseExpression();
    pop();  // list

    guardedAppend(list, expr);

    return list;
}

static void appendToAst(Value value) {
    if (IS_NIL(parser.ast)) {
        parser.ast = guardedCons(value, NIL_VAL);
    } else {
        guardedAppend(parser.ast, value);
    }
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.current->type != TOKEN_EOF) {
        if (parser.previous->type == TOKEN_RIGHT_PAREN) return;
        if (parser.current->type == TOKEN_LEFT_PAREN) return;
        parserAdvance();
    }
}
