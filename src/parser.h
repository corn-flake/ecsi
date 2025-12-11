
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
    EXPR_AND,
    EXPR_OR,
    EXPR_WHEN,
    EXPR_UNLESS,
    EXPR_BEGIN,

    // Unimplemented
    EXPR_LET,
    EXPR_COND,
    EXPR_LET_STAR,
    EXPR_CASE,
    EXPR_LETREC,
    EXPR_LETREC_STAR,
    EXPR_LET_VALUES,
    EXPR_LET_STAR_VALUES,
    EXPR_DO,
    EXPR_DELAY,
    EXPR_DELAY_FORCE,
    EXPR_PARAMETERIZE,
    EXPR_GUARD,
    EXPR_QUASIQUOTATION,
    EXPR_CASE_LAMBDA,
} ExprType;

typedef struct {
    ExprType type;
    SourceLocation location;
} Expr;

typedef SmartArray ExprPointerArray;

typedef struct {
    Expr expr;
    ExprPointerArray expressions;
} ExprSequence;

typedef ExprSequence ExprBegin;

typedef struct {
    Expr expr;
    ExprPointerArray definitions;
    ExprSequence *sequence;
} ExprBody;

typedef struct {
    Expr expr;
} ExprIdentifier;

typedef struct {
    Expr expr;
    Value value;
    bool isQuotation;
} ExprLiteral;

typedef struct {
    Expr expr;
    Expr *operator;
    ExprPointerArray operands;
} ExprCall;

typedef struct {
    Expr expr;
    ExprPointerArray formals;
    ExprPointerArray body;
} ExprLambda;

typedef struct {
    Expr expr;
    Expr *test;
    Expr *consequent;
    Expr *alternate;
} ExprIf;

typedef struct {
    Expr expr;
    ExprIdentifier *target;
    Expr *expression;
} ExprSet;

typedef struct {
    Expr expr;
    ExprPointerArray tests;
} ExprLogical;

typedef ExprLogical ExprOr;
typedef ExprLogical ExprAnd;

typedef struct {
    Expr expr;
    Expr *test;
    ExprSequence *sequence;
} ExprWhenUnless;

#define EXPR_TYPE(expr) ((expr)->type)
#define IS_IDENTIFIER(expr) (EXPR_IDENTIFIER == EXPR_TYPE(expr))
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

typedef ExprPointerArray AST;

typedef struct {
    Token previous;
    Token current;
    bool hadError;
    bool panicMode;
    AST ast;
} Parser;

extern Parser parser;

void initParser(void);
Expr *parseExpression(void);
Value parseDatum(void);
AST parseAllTokens(void);
void freeAST(AST ast);
