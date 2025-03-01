#ifndef clox_line_number_h
#define clox_line_number_h

typedef struct {
  int lineNumber;
  int repeats;
} LineNumber;

typedef struct {
  int count;
  int capacity;
  LineNumber* lineNumbers;
} LineNumberArray;

void initLineNumberArray(LineNumberArray* array);
void freeLineNumberArray(LineNumberArray* array);
void writeNumber(LineNumberArray* array, int lineNumber);

#endif
