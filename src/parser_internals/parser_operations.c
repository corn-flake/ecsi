#include "parser_operations.h"

#include <stdarg.h>
#include <stdio.h>

#include "../object.h"
#include "../parser.h"
#include "../value.h"
#include "../vm.h"

static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }
static bool tokensEqual(Token const *t1, Token const *t2);

void errorAt(Token const *token, char const *message) {
    formattedErrorAt(token, "%s", message);
}

void formattedErrorAt(Token const *token, char const *format, ...) {
    va_list args;
    va_start(args, format);
    varArgsFormattedErrorAt(token, format, args);
    va_end(args);
}

void error(char const *message) { errorAt(parser.previous, message); }

void formattedError(char const *format, ...) {
    va_list args;
    va_start(args, format);
    varArgsFormattedErrorAt(parser.previous, format, args);
    va_end(args);
}

void errorAtCurrent(char const *message) { errorAt(parser.current, message); }

void formattedErrorAtCurrent(char const *format, ...) {
    va_list args;
    va_start(args, format);
    varArgsFormattedErrorAt(parser.current, format, args);
    va_end(args);
}

void varArgsFormattedErrorAt(Token const *token, char const *format,
                             va_list args) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %zu] Error", token->line);
    if (TOKEN_EOF == token->type) {
        fprintf(stderr, " at end");
    } else if (TOKEN_ERROR == token->type) {
        /*
          We don't do anything if the token is an error because if it is,
          the token's error message will be passed as the MESSAGE
         */
    } else {
        fprintf(stderr, " at '%.*s'", (int)token->length, token->start);
    }
    fprintf(stderr, ": ");
    vfprintf(stderr, format, args);
    fputs("\n", stderr);

    parser.hadError = true;
}

void parserAdvance() {
    parser.previous = parser.current;

    while (true) {
        parser.current++;
        if (parser.current->type != TOKEN_ERROR) break;
        errorAt(parser.current, parser.current->start);
    }
}

void consume(TokenType type, char const *message) {
    if (type == parser.current->type) {
        parserAdvance();
        return;
    }
    errorAtCurrent(message);
}

bool check(TokenType type) { return type == parser.current->type; }

bool matchToken(Token const *token) {
    if (checkToken(token)) {
        parserAdvance();
        return true;
    }
    return false;
}

bool checkToken(Token const *token) {
    return tokensEqual(token, parser.current);
}

static bool tokensEqual(Token const *t1, Token const *t2) {
    return (t1->type == t2->type) && (t1->length == t2->length) &&
           !memcmp(t1->start, t2->start, t1->length);
}

bool parserMatch(TokenType type) {
    if (!check(type)) return false;
    parserAdvance();
    return true;
}

bool canContinueList() {
    return !check(TOKEN_RIGHT_PAREN) && !check(TOKEN_EOF);
}

bool tokenMatchesString(Token *token, char *const string) {
    return !memcmp(token->start, string, min(strlen(string), token->length));
}

bool currentTokenMatchesString(char *const string) {
    return tokenMatchesString(parser.current, string);
}

bool previousTokenMatchesString(char *const string) {
    return tokenMatchesString(parser.previous, string);
}

Value parseListUsing(ParseFn parse) { return parseNExprsIntoList(parse, -1); }

Value parseNExprsIntoList(ParseFn parse, int n) {
    if (0 == n) {
        consume(TOKEN_RIGHT_PAREN,
                "Expect right parenthesis to finish empty list");
        return NIL_VAL;
    }

    if (-1 == n) {
        return parseListUsing(parse);
    }

    Value expr = parse();

    Value list = guardedCons(expr, NIL_VAL);
    // We push here to avoid pushing in every iteration before calling parse.
    push(list);

    for (int i = 1; i < n; i++) {
        if (check(TOKEN_RIGHT_PAREN)) {
            formattedErrorAtCurrent(
                "Expected list of %d elements but only found %d elements.", n,
                i);
            break;
        }
        expr = parse();
        guardedAppend(list, expr);
    }

    pop();  // list
    consume(TOKEN_RIGHT_PAREN, "Expect ')' to close list.");

    return list;
}

Value parseListOfExpressions() { return parseListUsing(parseExpression); }

Value parseListOfDatums() { return parseListUsing(parseDatum); }
