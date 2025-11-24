
/*
  Copyright 2025 Evan Cooney

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

#include "scanner.h"
#include "value.h"

typedef enum {
    EXPR_IDENTIFIER,
    EXPR_LITERAL,
    EXPR_CALL,
    EXPR_LAMBDA,
    EXPR_IF,
    EXPR_SET,
    EXPR_COND,
    EXPR_AND,
    EXPR_OR,
    EXPR_WHEN,
    EXPR_UNLESS,
    EXPR_LET,

    // Unimplemented
    EXPR_LET_STAR,
    EXPR_CASE,
    EXPR_LETREC,
    EXPR_LETREC_STAR,
    EXPR_LET_VALUES,
    EXPR_LET_STAR_VALUES,
    EXPR_BEGIN,
    EXPR_DO,
    EXPR_DELAY,
    EXPR_DELAY_FORCE,
    EXPR_PARAMETERIZE,
    EXPR_GUARD,
    EXPR_QUASIQUOTATION,
    EXPR_CASE_LAMBDA,
} ASTNodeType;

typedef struct ASTNode {
    Token originatingToken;
    ASTNodeType type;
} ASTNode;

typedef struct {
    TokenArray tokens;
    Token *previous;
    Token *current;
    bool hadError;
    bool panicMode;
    // ASTNode ast;
    Value ast;
} Parser;

extern Parser parser;

void initParser(TokenArray tokens);
void markParserRoots();
Value parseExpression();
Value parseDatum();
Value parseAllTokens();
