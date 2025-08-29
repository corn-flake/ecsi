#include "parser_operations.h"

#include <stdio.h>

#include "../object.h"
#include "../parser.h"
#include "../value.h"
#include "../vm.h"

static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }

void errorAt(Token const *token, const char *message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %zu] Error", token->line);
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        /*
          We don't do anything if the token is an error because if it is,
          the token's error message will be passed as the MESSAGE
         */
    } else {
        fprintf(stderr, " at '%.*s'", (int)token->length, token->start);
    }
    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

void error(char const *message) { errorAt(parser.previous, message); }

void errorAtCurrent(char const *message) { errorAt(parser.current, message); }

void parserAdvance() {
    parser.previous = parser.current;

    while (true) {
        parser.current++;
        if (parser.current->type != TOKEN_ERROR) break;
        errorAt(parser.current, parser.current->start);
    }
}

void consume(TokenType type, char const *message) {
    if (parser.current->type == type) {
        parserAdvance();
        return;
    }
    errorAtCurrent(message);
}

bool check(TokenType type) { return parser.current->type == type; }

bool parserMatch(TokenType type) {
    if (!check(type)) return false;
    parserAdvance();
    return true;
}

bool canContinueList() {
    return !check(TOKEN_RIGHT_PAREN) && !check(TOKEN_EOF);
}

bool currentTokenMatchesString(char *const string) {
    return !memcmp(parser.current->start, string,
                   min(strlen(string), parser.current->length));
}

Value parseListUsing(ParseFn parse) {
    if (parserMatch(TOKEN_RIGHT_PAREN)) return NIL_VAL;

    Value expr = parse();

    push(expr);
    Value list = CONS(expr, NIL_VAL);
    pop();  // firstElement
    push(list);

    while (canContinueList()) {
        expr = parse();
        push(expr);
        append(AS_PAIR(list), expr);
        pop();  // expr
    }

    if (!parserMatch(TOKEN_RIGHT_PAREN)) {
        errorAtCurrent("Expect ')' to close list.");
    }

    pop();  // list
    return list;
}

Value parseListOfExpressions() { return parseListUsing(parseExpression); }

Value parseListOfDatums() { return parseListUsing(parseDatum); }
