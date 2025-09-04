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
bool tokenMatchesString(Token *token, char *const string);
bool previousTokenMatchesString(char *const string);
bool currentTokenMatchesString(char *const string);

Value parseListUsing(ParseFn parse);
// n == -1 indicates to parse until a right paren is found.
Value parseNExprsIntoList(ParseFn parse, int n);
Value parseListOfExpressions();
Value parseListOfDatums();
