#pragma once

#include <stdbool.h>

#include "../scanner.h"
#include "../value.h"

void advance();
void errorAt(Token const *token, char const *message);
void error(char const *message);
void errorAtCurrent(char const *message);
void consume(TokenType type, char const *message);
bool match(TokenType type);
bool check(TokenType type);
bool canContinueList();

Value parseListOfExpressions();
Value parseListOfDatums();
