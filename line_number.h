#pragma once

typedef struct {
  int lineNumber;
  int repeats;
} LineNumber;

typedef struct {
  int count;
  int capacity;
  LineNumber *lineNumbers;
} LineNumberArray;

void initLineNumberArray(LineNumberArray *array);
void freeLineNumberArray(LineNumberArray *array);
int writeNumber(LineNumberArray *array, int lineNumber);
int *decompressLineNumberArray(LineNumberArray *array);
int numberOfEntries(LineNumberArray *array);
