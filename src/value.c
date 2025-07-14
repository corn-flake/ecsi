#include "value.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"

static char *booleanToString(bool b);
static char *doubleToString(double d);
static char *longToString(long l);
static size_t numberOfDigitsInLong(long l);
static long fractionToWholeNumber(double fraction);
static bool doubleIsWholeNumber(double d);

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
      return false;  // Unreachable
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
    char *nilString = checkedMalloc(4);
    memcpy(nilString, "nil", 3);
    nilString[3] = '\0';
    return nilString;
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
      break;
    case VAL_NIL: {
      char *nilString = checkedMalloc(4);
      memcpy(nilString, "nil", 3);
      nilString[3] = '\0';
      return nilString;
      break;
    }
    case VAL_NUMBER:
      return doubleToString(AS_NUMBER(value));
      break;
    case VAL_OBJ:
      return objectToString(value);
      break;
    default:
      return "";
  }
#endif
}

static char *doubleToString(double d) {
  long integerPart = (long)d;
  double fractionalPart = d - (double)(long)d;
  long fractionalPartAsWhole = fractionToWholeNumber(fractionalPart);

  size_t integerPartDigits = numberOfDigitsInLong(integerPart);
  size_t fractionalPartDigits = numberOfDigitsInLong(fractionalPartAsWhole);

  // Add 2 for the dot and the null terminator
  char *string = checkedMalloc(integerPartDigits + fractionalPartDigits + 2);
  char *integerPartString = longToString(integerPart);
  char *fractionalPartString = longToString(fractionalPartAsWhole);
  char *positionInString = string;

  memcpy(positionInString, integerPartString, integerPartDigits);
  positionInString += integerPartDigits;

  *positionInString = '.';
  positionInString++;

  memcpy(positionInString, fractionalPartString, fractionalPartDigits);
  positionInString += fractionalPartDigits;

  *positionInString = '\0';

  free(integerPartString);
  free(fractionalPartString);
  return string;
}

static char *longToString(long l) {
#define INTEGRAL_TO_CHAR(x) (char)(48 + x)
  size_t length = numberOfDigitsInLong(l);
  char *string = checkedMalloc(length + 1);

  size_t i = length - 1;
  do {
    string[i] = INTEGRAL_TO_CHAR(l % 10L);
    l /= 10L;
  } while (i--);

  string[length] = '\0';
  return string;
#undef INTEGRAL_TO_CHAR
}

static bool doubleIsWholeNumber(double d) { return d == (double)(long)d; }

static long fractionToWholeNumber(double fraction) {
  while (!doubleIsWholeNumber(fraction)) {
    fraction *= 10;
  }
  return fraction;
}

static size_t numberOfDigitsInLong(long l) { return (size_t)ceil(log10(l)); }

static char *booleanToString(bool b) {
  char *string = NULL;
  if (b) {
    string = ALLOCATE(char, 5);
    memcpy(string, "true", 4);
    string[4] = '\0';
  } else {
    string = ALLOCATE(char, 6);
    memcpy(string, "false", 5);
    string[5] = '\0';
  }
  return string;
}

void printValue(Value value) {
#ifdef NAN_BOXING
  if (IS_BOOL(value)) {
    printf(AS_BOOL(value) ? "true" : "false");
  } else if (IS_NIL(value)) {
    printf("nil");
  } else if (IS_NUMBER(value)) {
    printf("%g", AS_NUMBER(value));
  } else if (IS_OBJ(value)) {
    printObject(value);
  }
#else
  switch (value.type) {
    case VAL_BOOL:
      printf(AS_BOOL(value) ? "true" : "false");
      break;
    case VAL_NIL:
      printf("nil");
      break;
    case VAL_NUMBER:
      printf("%g", AS_NUMBER(value));
      break;
    case VAL_OBJ:
      printObject(value);
      break;
  }
#endif
}
