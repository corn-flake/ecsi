#include <stdlib.h>

#include "line_number.h"
#include "memory.h"

void initLineNumberArray(LineNumberArray *array) {
  array->count = 0;
  array->capacity = 0;
  array->lineNumbers = NULL;
}

void writeNumber(LineNumberArray *array, int lineNumber) {
  if (array->count > 0 &&
      array->lineNumbers[array->count - 1].lineNumber == lineNumber) {
    array->lineNumbers[array->count - 1].repeats++;
    return;
  }

  if (array->capacity < array->count + 1) {
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);
    array->lineNumbers = GROW_ARRAY(LineNumber, array->lineNumbers, oldCapacity,
                                    array->capacity);
  }

  LineNumber encoded_number = {
      .lineNumber = lineNumber,
      .repeats = 1,
  };
  array->lineNumbers[array->count] = encoded_number;
  array->count++;
}

void freeLineNumberArray(LineNumberArray *array) {
  FREE_ARRAY(LineNumber, array->lineNumbers, array->capacity);
  initLineNumberArray(array);
}
