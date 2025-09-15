/*
  Copyright 2025 Evan Cooney
  Copyright 2015-2020 Robert Nystrom

  This file is part of Ecsi.

  Ecsi is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  Ecsi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  Ecsi. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "object.h"

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_LAMBDA,
    TOKEN_IF,
    TOKEN_SET,
    TOKEN_COND,
    TOKEN_DEFINE,
    TOKEN_LET,
    TOKEN_AND,
    TOKEN_ELSE,
    TOKEN_OR,
    TOKEN_WHEN,
    TOKEN_UNLESS,
    TOKEN_BEGIN,
    TOKEN_BOOLEAN,
    TOKEN_NUMBER,
    TOKEN_CHARACTER,
    TOKEN_STRING,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_POUND_LEFT_PAREN,
    TOKEN_POUND_U8_LEFT_PAREN,
    TOKEN_QUOTE,
    TOKEN_BACKQUOTE,
    TOKEN_COMMA,
    TOKEN_COMMA_AT,
    TOKEN_PERIOD,
    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

char const *tokenTypeToString(TokenType type);

typedef struct {
    TokenType type;
    char const *start;
    size_t length;
    size_t line;
} Token;

void printToken(Token *token);
ObjString *tokenToObjString(Token *token);
ObjSymbol *tokenToObjSymbol(Token *token);

typedef struct {
    size_t count;
    size_t capacity;
    Token *array;
} TokenArray;

void initTokenArray(TokenArray *tokenArray);
void addToken(TokenArray *tokenArray, Token token);
void freeTokenArray(TokenArray *tokenArray);

typedef struct {
    char const *start;
    char const *current;
    size_t line;
} Scanner;

extern Scanner scanner;

void initScanner(char const *source);
Token scanToken();
void scanAllTokensInto(TokenArray *tokenArray);
