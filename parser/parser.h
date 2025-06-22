#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "../scanner/scanner.h"
#include "../value.h"

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
void parserAdvance();
Value parseDatum();
Value parseExpression();
void markParserRoots();
void errorAt(Token *token, const char *message);
void error(const char *message);
void errorAtCurrent(const char *message);
void consume(TokenType type, const char *message);
bool parserMatch(TokenType type);
bool check(TokenType type);
Value parseList(bool parseDatums);
