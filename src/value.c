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

#include "value.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"

// Return heap-allocated string representation of b.
static char *booleanToString(bool b);

// Return heap-allocated string representation of d.
static char *doubleToString(double d);

// Return heap-allocated string representation of l.
// Calculate number of digits in l.
static size_t numberOfDigitsInLong(long l);

// Convert a fractional value into an integer value with the same digits.
static long fractionToWholeNumber(double fraction);

/*
  Duplicate S as a heap-allocated string, and die with an appropriate message if
  allocation fails.
*/
static char *checkedStrdup(char const *s);

bool valuesEqual(Value a, Value b) {
#ifdef NAN_BOXING
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        return AS_NUMBER(a) == AS_NUMBER(b);
    }
    return a == b;
#else
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:
            return true;
        case VAL_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:
            return AS_OBJ(a) == AS_OBJ(b);
        default:
            UNREACHABLE();
    }
#endif
}

void initValueArray(ValueArray *array) {
    array->values = NULL;
    array->capacity = array->count = 0;
}

void writeValueArray(ValueArray *array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values =
            GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count++] = value;
}

void freeValueArray(ValueArray *array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValueArray(ValueArray *array) {
    for (int i = 0; i < array->count; i++) {
        printValue(array->values[i]);

        // Don't print a space on the final iteration.
        if (i != array->count - 1) {
            putchar(' ');
        }
    }
}

char *valueToString(Value value) {
#ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        return booleanToString(AS_BOOL(value));
    } else if (IS_NIL(value)) {
        return checkedStrdup("nil");
    } else if (IS_NUMBER(value)) {
        return doubleToString(AS_NUMBER(value));
    } else if (IS_OBJ(value)) {
        return objectToString(value);
    } else {
        return "";
    }
#else
    switch (value.type) {
        case VAL_BOOL:
            return booleanToString(AS_BOOL(value));
        case VAL_NIL:
            return checkedStrdup("nil");
        case VAL_NUMBER:
            return doubleToString(AS_NUMBER(value));
        case VAL_OBJ:
            return objectToString(value);
        default:
            return "";
    }
#endif
}

static char *doubleToString(double d) {
    long integerPart = trunc(d);
    long fractionalPart = fractionToWholeNumber(d - (double)integerPart);
    size_t stringLength = numberOfDigitsInLong(integerPart) +
                          numberOfDigitsInLong(fractionalPart);
    // Add 2 for the null terminator and the dot.
    char *string = checkedMalloc(stringLength + 2);
    snprintf(string, stringLength, "%ld.%ld", integerPart, fractionalPart);
    return string;
}

bool doubleIsInteger(double d) { return trunc(d) == d; }

static long fractionToWholeNumber(double fraction) {
    while (!doubleIsInteger(fraction)) {
        fraction *= 10;
    }
    return fraction;
}

static size_t numberOfDigitsInLong(long l) { return (size_t)ceil(log10(l)); }

static char *booleanToString(bool b) {
    return checkedStrdup(b ? "#true" : "#false");
}

static char *checkedStrdup(char const *s) {
    char *string = strdup(s);
    if (NULL == string) {
        DIE("Failed to duplicate \"%s\"", s);
    }
    return string;
}

void printValue(Value value) {
#ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        printf(AS_BOOL(value) ? "#true" : "#false");
    } else if (IS_NIL(value)) {
        printf("nil");
    } else if (IS_NUMBER(value)) {
        printf("%g", AS_NUMBER(value));
    } else if (IS_CHARACTER(value)) {
        putchar(AS_CHARACTER(value));
    } else if (IS_OBJ(value)) {
        printObject(value);
    }
#else
    switch (value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "#true" : "#false");
            break;
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VAL_CHARACTER:
            putchar(AS_CHARACTER(value));
            break;
        case VAL_OBJ:
            printObject(value);
            break;
    }
#endif
}
