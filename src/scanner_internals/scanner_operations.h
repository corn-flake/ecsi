#pragma once

#include "../scanner.h"

char peek();
char peekNext();
char advance();
bool match(char expected);
bool matchString(const char *string);
Token makeToken(TokenType type);
Token errorToken(const char *message);
bool isAtEnd();
