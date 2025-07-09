#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "object.h"

typedef enum {
  TOKEN_IDENTIFIER,
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

const char *tokenTypeToString(TokenType type);

typedef struct {
  TokenType type;
  const char *start;
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

void initTokenArray(TokenArray *token_array);
void addToken(TokenArray *token_array, Token token);
void freeTokenArray(TokenArray *token_array);

typedef struct {
  const char *start;
  const char *current;
  size_t line;
} Scanner;

extern Scanner scanner;

void initScanner(const char *source);
Token scanToken();
