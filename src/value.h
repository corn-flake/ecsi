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

#include <math.h>
#include <string.h>

#include "common.h"
#include "smart_array.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

#include <limits.h>

#define SIGN_BIT ((uint64_t)0x8000000000000000)

#define QNAN ((uint64_t)0x7ffc000000000000)

#define TAG_NIL 1    // 01.
#define TAG_FALSE 2  // 10.
#define TAG_TRUE 3   // 11.

typedef uint64_t Value;

#define BOOL_VAL(b) ((b) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL ((Value)(uint64_t)(QNAN | TAG_NIL))
#define NUMBER_VAL(num) numToValue(num)
#define OBJ_VAL(obj) (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))
#define CHARACTER_VAL(character) (NUMBER_VAL((double)(character)))

static inline Value numToValue(double num) {
    Value value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

static inline double valueToNum(Value value) {
    double num;
    memcpy(&num, &value, sizeof(Value));
    return num;
}

#define IS_BOOL(value) (((value) | 1) == TRUE_VAL)
#define IS_NIL(value) ((value) == NIL_VAL)
#define IS_NUMBER(value) (((value) & QNAN) != QNAN)
#define IS_OBJ(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_NUMBER(value) valueToNum(value)
#define AS_OBJ(value) ((Obj *)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))
#define AS_CHARACTER(value) ((char)AS_NUMBER(value))

static inline bool isCharacter(Value value) {
    return IS_EXACT_INTEGER(value) && CHAR_MIN <= AS_NUMBER(value) &&
           CHAR_MAX >= AS_NUMBER(value);
}

#define IS_CHARACTER(value) isCharacter(value)

#else

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_CHARACTER,
    VAL_OBJ,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        char character;
        Obj *obj;
    } as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_CHARACTER(value) ((value).type == VAL_CHARACTER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define AS_OBJ(value) ((value).as.obj)
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_CHARACTER(value) ((value).as.character)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = (value)}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = (value)}})
#define CHARACTER_VAL(value) ((Value){VAL_CHARACTER, {.character = (value)}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj *)(object)}})

#endif

bool doubleIsInteger(double d);

static inline bool isExactInteger(Value value) {
    return IS_NUMBER(value) && doubleIsInteger(AS_NUMBER(value));
}

#define IS_EXACT_INTEGER(value) isExactInteger(value)

/*
typedef struct {
    int count, capacity;
    Value *values;
} ValueArray;
*/

typedef SmartArray ValueArray;

// Test if two values are equal.
bool valuesEqual(Value a, Value b);

/*
  Returns a heap-allocated string of the string representation
  of value.
 */
char *valueToString(Value value);

// Print a value.
void printValue(Value value);

// Initialize the ValueArray at array.
void initValueArray(ValueArray *array);

// Append value to the ValueArray at array.
void writeValueArray(ValueArray *array, Value value);

// Get the element at index index in a ValueArray
Value getValueArrayAt(ValueArray const *array, size_t index);

// Set the element at index in array to value.
void setValueArrayAt(ValueArray *array, size_t index, Value value);

// Get the number of elements in array.
size_t getValueArrayCount(ValueArray const *array);

// Free memory associated with the ValueArray at array.
void freeValueArray(ValueArray *array);

// Print the ValueArray at array.
void printValueArray(ValueArray *array);
