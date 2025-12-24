
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

typedef SmartArray ObjSyntaxPointerArray;

// Redefine all these macros
#define IS_IDENTIFIER(expr) (true)
#define IS_CALL(expr) (EXPR_CALL == EXPR_TYPE(expr))
#define IS_LAMBDA(expr) (EXPR_LAMBDA == EXPR_TYPE(expr))
#define IS_IF(expr) (EXPR_IF == EXPR_TYPE(expr))
#define IS_SET(expr) (EXPR_SET == EXPR_TYPE(expr))
#define IS_AND(expr) (EXPR_AND == EXPR_TYPE(expr))
#define IS_OR(expr) (EXPR_OR == EXPR_TYPE(expr))

#define AS_IDENTIFIER(expr) ((ExprIdentifier *)(expr))
#define AS_CALL(expr) ((ExprCall *)(expr))
#define AS_LAMBDA(expr) ((ExprLambda *)(expr))
#define AS_IF(expr) ((ExprIf *)(expr))
#define AS_SET(expr) ((ExprSet *)(expr))
#define AS_AND(expr) ((ExprAnd *)(expr))
#define AS_OR(expr) ((ExprOr *)(expr))

typedef struct {
    Token previous;
    Token current;
    bool hadError;
    bool panicMode;
    ObjSyntaxPointerArray ast;
} Parser;

extern Parser parser;

void initParser(void);
ObjSyntax *parseExpression(void);
Value parseDatum(void);
ObjSyntaxPointerArray parseAllTokens(void);
void printAST(ObjSyntaxPointerArray const *ast);
void freeAST(ObjSyntaxPointerArray *ast);
