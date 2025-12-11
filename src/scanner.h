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

// A slice of source code.
typedef struct {
    size_t line;    // The line the code is on.
    size_t length;  // The number of characters in the slice.
    // The address of the first character of the slice in the
    // source code string.
    char const *start;
} SourceLocation;

// The type of a token.
typedef enum {
    // Keywords
    TOKEN_AND,
    TOKEN_BEGIN,
    TOKEN_CASE,
    TOKEN_CASE_LAMBDA,
    TOKEN_COND,
    TOKEN_DEFINE,
    TOKEN_DELAY,
    TOKEN_DELAY_FORCE,
    TOKEN_DO,
    TOKEN_ELSE,
    TOKEN_GUARD,
    TOKEN_IF,
    TOKEN_LAMBDA,
    TOKEN_LET,
    TOKEN_LETREC,
    TOKEN_LETREC_STAR,
    TOKEN_LET_STAR,
    TOKEN_LET_STAR_VALUES,
    TOKEN_LET_VALUES,
    TOKEN_OR,
    TOKEN_PARAMETERIZE,
    TOKEN_SET,
    TOKEN_UNLESS,
    TOKEN_WHEN,

    TOKEN_IDENTIFIER,

    // Literals
    TOKEN_BOOLEAN,
    TOKEN_NUMBER,
    TOKEN_CHARACTER,
    TOKEN_STRING,

    TOKEN_POUND_LEFT_PAREN,
    TOKEN_POUND_U8_LEFT_PAREN,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_QUOTE,
    TOKEN_BACKQUOTE,
    TOKEN_COMMA,
    TOKEN_COMMA_AT,
    TOKEN_PERIOD,
    TOKEN_LEFT_ARROW,

    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

// Return a string literal representation of type.
char const *tokenTypeToString(TokenType type);

// A token of code.
typedef struct {
    TokenType type;           // Token type
    SourceLocation location;  // Location in source code;
} Token;

// Get the line of a token
size_t tokenGetLine(Token const *token);
size_t tokenGetLength(Token const *token);
char const *tokenGetStart(Token const *token);
TokenType tokenGetType(Token const *token);

// Print a token
void printToken(Token const *token);

// Return a ObjString representation of token's text.
ObjString *tokenToObjString(Token const *token);

// Return an ObjSymbol representation of token's text.
ObjSymbol *tokenToObjSymbol(Token const *token);

// The state of the scanner
typedef struct {
    char const *start;    // Pointer into the source code to the start of the
                          // current token
    char const *current;  // Pointer into the source code to the current
                          // character being considered
    size_t line;          // The current line
} Scanner;

// Global scanner variable, declared in scanner.c
extern Scanner scanner;

// Initialize the scanner with a source code string.
void initScanner(char const *source);

// Scan and return one token.
Token scanToken(void);
