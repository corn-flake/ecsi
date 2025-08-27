#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "scanner.h"
#include "value.h"

typedef struct {
    TokenArray tokens;
    Token *previous;
    Token *current;
    bool hadError;
    bool panicMode;
    Value ast;
} Parser;

extern Parser parser;

void initParser(TokenArray tokens);
void markParserRoots();
Value parseExpression();
Value parseDatum();
Value parseAllTokens();
