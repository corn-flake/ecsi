#include "scanner.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "scanner_internals/character_type_tests.h"
#include "scanner_internals/identifier.h"
#include "scanner_internals/intertoken_space.h"
#include "scanner_internals/pound_something.h"
#include "scanner_internals/scanner_operations.h"

Scanner scanner;

char const *tokenTypeToString(TokenType type) {
    switch (type) {
        case TOKEN_IDENTIFIER:
            return "TOKEN_IDENTIFIER";
        case TOKEN_NUMBER:
            return "TOKEN_NUMBER";
        case TOKEN_STRING:
            return "TOKEN_STRING";
        case TOKEN_LEFT_PAREN:
            return "TOKEN_LEFT_PAREN";
        case TOKEN_RIGHT_PAREN:
            return "TOKEN_RIGHT_PAREN";
        case TOKEN_ERROR:
            return "TOKEN_ERROR";
        case TOKEN_PERIOD:
            return "TOKEN_PERIOD";
        case TOKEN_COMMA:
            return "TOKEN_COMMA";
        case TOKEN_BACKQUOTE:
            return "TOKEN_BACKQUOTE";
        case TOKEN_COMMA_AT:
            return "TOKEN_COMMA_AT";
        case TOKEN_BOOLEAN:
            return "TOKEN_BOOLEAN";
        case TOKEN_POUND_LEFT_PAREN:
            return "TOKEN_POUND_LEFT_PAREN";
        case TOKEN_POUND_U8_LEFT_PAREN:
            return "TOKEN_POUND_U8_LEFT_PAREN";
        case TOKEN_CHARACTER:
            return "TOKEN_CHARACTER";
        case TOKEN_EOF:
            return "TOKEN_EOF";
        default:
            // Unreached.
            fprintf(stderr, "tokenTypeToString (%d) - Unknown token type.\n",
                    type);
            assert(false);
    }
}

void printToken(Token *token) {
    printf("Token { .type = %s, .start = '%.*s', .line = %zu }\n",
           tokenTypeToString(token->type), (int)token->length, token->start,
           token->line);
}

ObjString *tokenToObjString(Token *token) {
    return copyString(token->start, token->length);
}

ObjSymbol *tokenToObjSymbol(Token *token) {
    return newSymbol(token->start, token->length);
}

void initTokenArray(TokenArray *tokenArray) {
    tokenArray->count = 0;
    tokenArray->capacity = 0;
    tokenArray->array = NULL;
}

void addToken(TokenArray *tokenArray, Token token) {
    if (tokenArray->count >= tokenArray->capacity) {
        size_t oldCapacity = tokenArray->capacity;
        tokenArray->capacity = GROW_CAPACITY(oldCapacity);
        tokenArray->array = GROW_ARRAY(Token, tokenArray->array, oldCapacity,
                                       tokenArray->capacity);
    }
    tokenArray->array[tokenArray->count] = token;
    tokenArray->count++;
}

void freeTokenArray(TokenArray *tokenArray) {
    FREE_ARRAY(Token, tokenArray->array, tokenArray->capacity);
    tokenArray->capacity = 0;
    tokenArray->count = 0;
}

void initScanner(char const *source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static Token number() {
    while (isdigit(peek())) advance();

    return makeToken(TOKEN_NUMBER);
}

static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    // The closing quote
    advance();
    return makeToken(TOKEN_STRING);
}

Token scanToken() {
    skipIntertokenSpace();
    scanner.start = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    if (isdigit(c)) return number();

    if (isInitial(c)) return identifier(IDENTIFIER_NO_VERTICAL_LINE);

    if (isExplicitSign(c)) {
        return peculiarIdentifier(IDENTIFIER_PECULIAR_NO_DOT);
    }

    switch (c) {
        case '(':
            return makeToken(TOKEN_LEFT_PAREN);
        case ')':
            return makeToken(TOKEN_RIGHT_PAREN);
        case '\'':
            return makeToken(TOKEN_QUOTE);
        case '`':
            return makeToken(TOKEN_BACKQUOTE);
        case ',':
            if (match('@')) {
                return makeToken(TOKEN_COMMA_AT);
            }
            return makeToken(TOKEN_COMMA);
        case '.':
            if (isDotSubsequent(peek())) {
                return peculiarIdentifier(IDENTIFIER_PECULIAR_STARTS_WITH_DOT);
            }
            return makeToken(TOKEN_PERIOD);
        case '"':
            return string();
        case '|':
            return identifier(IDENTIFIER_STARTS_WITH_VERTICAL_LINE);
        case '#':
            return poundSomething();
    }

    return errorToken("Unexpected character.");
}

void scanAllTokensInto(TokenArray *tokenArray) {
    Token token;
    do {
        token = scanToken();
        addToken(tokenArray, token);
    } while (TOKEN_EOF != token.type);

    /*
    Token token = scanToken();
    while (token.type != TOKEN_EOF) {
        addToken(tokenArray, token);
        token = scanToken();
    }
    // Add EOF token
    addToken(tokenArray, token);
    */
}
