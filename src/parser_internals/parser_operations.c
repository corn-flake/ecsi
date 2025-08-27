#include "parser_operations.h"

#include <stdio.h>

#include "../object.h"
#include "../parser.h"
#include "../value.h"
#include "../vm.h"

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

void advance() {
    parser.previous = parser.current;

    while (true) {
        parser.current++;
        if (parser.current->type != TOKEN_ERROR) break;
        errorAt(parser.current, parser.current->start);
    }
}

void consume(TokenType type, char const *message) {
    if (parser.current->type == type) {
        advance();
        return;
    }
    errorAtCurrent(message);
}

bool check(TokenType type) { return parser.current->type == type; }

bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

bool canContinueList() {
    return !check(TOKEN_RIGHT_PAREN) && !check(TOKEN_EOF);
}

static Value parseList(bool parseDatums) {
    if (check(TOKEN_RIGHT_PAREN)) return NIL_VAL;

    Value firstElement = parseDatums ? parseDatum() : parseExpression();

    push(firstElement);
    ObjPair *list = newPair(firstElement, NIL_VAL);
    pop();  // firstElement

    push(OBJ_VAL(list));
    while (!check(TOKEN_RIGHT_PAREN)) {
        append(list, parseDatums ? parseDatum() : parseExpression());
    }
    pop();  // list

    return OBJ_VAL(list);
}

Value parseListOfExpressions() { return parseList(false); }

Value parseListOfDatums() { return parseList(true); }
