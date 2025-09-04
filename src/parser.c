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

    return value;
}

static Value parseListBasedExpression() {
    ParseFn parse = getDerivedExpressionParseFn();

    // Derived expression
    if (NULL != parse) {
        Value name = symbol();
        push(name);

        Value expr = CONS(name, NIL_VAL);
        pop();  // name
        push(expr);

        Value rest = parse();
        push(rest);

        SET_CDR(expr, rest);

        pop();  // rest
        pop();  // expr
        return expr;
    }

    // Procedure call
    return parseListOfExpressions();
}

static Value parseQuotation() {
    size_t const QUOTE_LEN = 5;
    Value quoteSymbol = OBJ_VAL(newSymbol("quote", QUOTE_LEN));
    push(quoteSymbol);

    Value list = CONS(quoteSymbol, NIL_VAL);
    pop();  // quoteSymbol
    push(list);

    Value expr = parseExpression();
    push(expr);
    append(AS_PAIR(list), expr);

    pop();  // expr
    pop();  // list
    return list;
}

Value parseDatum() {
    Value value = NIL_VAL;

    switch (parser.current->type) {
            // Simple datums.
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
        case TOKEN_IDENTIFIER:
            value = symbol();
            parserAdvance();
            break;
        case TOKEN_POUND_U8_LEFT_PAREN:
            value = parseBytevector();
            break;

            // Compound datums.
        case TOKEN_LEFT_PAREN:
            // Consume the '('
            parserAdvance();
            value = parseListOfDatums();
            break;
        case TOKEN_POUND_LEFT_PAREN:
            // Consume the '#('
            parserAdvance();
            value = parseVectorUsing(parseDatum);
            break;
        case TOKEN_QUOTE:
        case TOKEN_BACKQUOTE:
        case TOKEN_COMMA:
        case TOKEN_COMMA_AT:
            parserAdvance();
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
