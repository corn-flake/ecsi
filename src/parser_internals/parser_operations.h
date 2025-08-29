#pragma once

#include <stdbool.h>

#include "../scanner.h"
#include "../value.h"

typedef Value (*ParseFn)(void);

void parserAdvance();
void errorAt(Token const *token, char const *message);
void error(char const *message);
void errorAtCurrent(char const *message);
void consume(TokenType type, char const *message);
bool parserMatch(TokenType type);
bool check(TokenType type);
bool canContinueList();
bool currentTokenMatchesString(char *const string);

Value parseListUsing(ParseFn parse);
Value parseListOfExpressions();
Value parseListOfDatums();
