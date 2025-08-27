#pragma once

#include "../scanner.h"

char peek();
char peekNext();
char advance();
bool match(char expected);
bool matchString(char const *string);
Token makeToken(TokenType type);
Token errorToken(char const *message);
bool isAtEnd();
