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

static Value listBasedExpression();
static void appendToAst(Value value);

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
    while (parser.current < parser.tokens.array + parser.tokens.count) {
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
            value = BOOL_VAL(booleanTokenToBool(parser.current));
            advance();
            break;
        case TOKEN_NUMBER:
            value = NUMBER_VAL(numberTokenToDouble(parser.current));
            advance();
            break;
        case TOKEN_CHARACTER:
            value = CHARACTER_VAL(characterTokenToChar(parser.current));
            break;
        case TOKEN_STRING:
            value = OBJ_VAL(tokenToObjString(parser.current));
            break;
        case TOKEN_POUND_LEFT_PAREN:
            value = vector(false);
            break;
        case TOKEN_POUND_U8_LEFT_PAREN:
            value = bytevector();
            break;

            // Quotation
        case TOKEN_QUOTE:
            // Read the quote
            advance();
            value = parseDatum();
            break;

            // Identifiers
        case TOKEN_IDENTIFIER:
            value = symbol();
            break;

        case TOKEN_LEFT_PAREN: {
            value = listBasedExpression();
            break;
        }

        default:
            fprintf(stderr, "TODO: parse %s tokens.\n",
                    tokenTypeToString(parser.current->type));
    }

    return value;
}

Value parseDatum() {
    Value value = NIL_VAL;

    switch (parser.current->type) {
            // Simple datums.
        case TOKEN_BOOLEAN:
            value = BOOL_VAL(booleanTokenToBool(parser.current));
            advance();
            break;
        case TOKEN_NUMBER:
            value = NUMBER_VAL(numberTokenToDouble(parser.current));
            advance();
            break;
        case TOKEN_CHARACTER:
            value = CHARACTER_VAL(characterTokenToChar(parser.current));
            advance();
            break;
        case TOKEN_STRING:
            value = OBJ_VAL(tokenToObjString(parser.current));
            advance();
            break;
        case TOKEN_IDENTIFIER:
            value = symbol();
            advance();
            break;
        case TOKEN_POUND_U8_LEFT_PAREN:
            value = bytevector();
            break;

            // Compound datums.
        case TOKEN_LEFT_PAREN:
            // Consume the '('
            advance();
            value = parseListOfDatums();
            break;
        case TOKEN_POUND_LEFT_PAREN:
            // Consume the '#('
            advance();
            value = vector(true);
            break;
        case TOKEN_QUOTE:
        case TOKEN_BACKQUOTE:
        case TOKEN_COMMA:
        case TOKEN_COMMA_AT:
            advance();
            value = parseDatum();
            break;
        default:
            value = NIL_VAL;
    }

    return value;
}

static void appendToAst(Value value) {
    push(value);
    if (IS_NIL(parser.ast)) {
        parser.ast = CONS(value, NIL_VAL);
    } else {
        append(AS_PAIR(parser.ast), value);
    }
    pop();  // value
}

static Value listBasedExpression() {
    // Read the left paren
    advance();

    if (TOKEN_RIGHT_PAREN == parser.current->type) {
        return NIL_VAL;  // Empty list
    }

    Value expr = parseExpression();
    push(expr);

    Value list = CONS(expr, NIL_VAL);
    pop();  // expr
    push(list);

    while (TOKEN_RIGHT_PAREN != parser.current->type) {
        expr = parseExpression();
        push(expr);
        append(AS_PAIR(list), expr);
        pop();  // expr
    }

    advance();  // Read closing paren

    pop();  // list
    return list;
}
