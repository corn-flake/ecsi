#include <stdlib.h>

#include "chunk.h"
#include "memory.h"
#include "value.h"
#include "line_number.h"

void initChunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  initValueArray(&chunk->constants);
  initLineNumberArray(&chunk->lines);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  writeNumber(&chunk->lines, line);
  chunk->count++;
}

void freeChunk(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  freeLineNumberArray(&chunk->lines);
  freeValueArray(&chunk->constants);
  initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value) {
  writeValueArray(&chunk->constants, value);
  return chunk->constants.count - 1;
}

int getLine(Chunk* chunk, int offset) {
  int currOffset = 0;
  int currLineIndex = 0;
  while (currOffset < offset && currLineIndex < chunk->lines.count) {
    if (chunk->lines.lineNumbers[currLineIndex].repeats >= offset + 1) {
      return chunk->lines.lineNumbers[currLineIndex].lineNumber;
    }
    currOffset += chunk->lines.lineNumbers[currLineIndex].repeats;
    currLineIndex++;
  }
  return chunk->lines.lineNumbers[currLineIndex].lineNumber;
}

void writeConstant(Chunk* chunk, Value value, int line) {
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
