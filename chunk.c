#include "chunk.h"

#include <assert.h>
#include <stdlib.h>

#include "line_number.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

void initChunk(Chunk *chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  initValueArray(&chunk->constants);
  initLineNumberArray(&chunk->lines);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code =
        GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  assert(line == writeNumber(&chunk->lines, line));
  chunk->count++;
}

void freeChunk(Chunk *chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  freeLineNumberArray(&chunk->lines);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

int addConstant(Chunk *chunk, Value value) {
  push(value);
  writeValueArray(&chunk->constants, value);
  pop();
  return chunk->constants.count - 1;
}

int getLine(Chunk *chunk, int offset) {
  int counter = 0;
  int entryIndex = 0;
  int entriesCount = numberOfEntries(&chunk->lines);

  for (entryIndex = 0; entryIndex < entriesCount; entryIndex++) {
    LineNumber currentEntry = chunk->lines.lineNumbers[entryIndex];
    if (counter + currentEntry.repeats >= offset) {
      return currentEntry.lineNumber;
    } else {
      counter += currentEntry.repeats;
    }
  }

  return chunk->lines.lineNumbers[entryIndex].lineNumber;
}

void writeConstant(Chunk *chunk, Value value, int line) {
#define NTH_BYTE(number, n) ((number >> (8 * n)) & 0xFF)
  int constant_index = addConstant(chunk, value);
  if (constant_index >= 256) {
    writeChunk(chunk, OP_CONSTANT_LONG, line);
    writeChunk(chunk, NTH_BYTE(constant_index, 2), line);
    writeChunk(chunk, NTH_BYTE(constant_index, 1), line);
    writeChunk(chunk, NTH_BYTE(constant_index, 0), line);
  } else {
    writeChunk(chunk, OP_CONSTANT, line);
    writeChunk(chunk, (uint8_t)constant_index, line);
  }
#undef NTH_BYTE
}
