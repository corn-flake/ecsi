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

#include "chunk.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "line_number.h"
#include "memory.h"
#include "smart_array.h"
#include "value.h"
#include "vm.h"

void initChunk(Chunk *chunk) {
    initSmartArray(&(chunk->code), reallocate, sizeof(uint8_t));
    initValueArray(&chunk->constants);
    initLineNumberArray(&chunk->lines);
}

void writeChunk(Chunk *chunk, uint8_t byte, unsigned int line) {
    smartArrayAppend(&(chunk->code), &byte);
    unsigned int writtenLine = writeNumber(&chunk->lines, line);
    assert(line == writtenLine);
}

uint8_t getChunkAt(Chunk const *chunk, size_t index) {
    return SMART_ARRAY_AT(&(chunk->code), index, uint8_t);
}

void setChunkAt(Chunk *chunk, size_t index, uint8_t byte) {
    SMART_ARRAY_AT(&(chunk->code), index, uint8_t) = byte;
}

uint8_t *getChunkCode(Chunk const *chunk) {
    return &(SMART_ARRAY_AT(&(chunk->code), 0, uint8_t));
}

size_t getChunkCount(Chunk const *chunk) {
    return getSmartArrayCount(&(chunk->code));
}

void freeChunk(Chunk *chunk) {
    freeSmartArray(&(chunk->code));
    freeLineNumberArray(&(chunk->lines));
    freeValueArray(&(chunk->constants));
    initChunk(chunk);
}

int addConstant(Chunk *chunk, Value value) {
    push(value);
    writeValueArray(&(chunk->constants), value);
    pop();
    return getValueArrayCount(&(chunk->constants) - 1);
}

int getLine(Chunk *chunk, int offset) {
    // TODO: Change offset to a size_t
    assert(offset >= 0);

    size_t entriesCount = numberOfEntries(&(chunk->lines));
    /*
      No entries indicates no data in the chunk, and that's a
      chunk state that getLine shouldn't be called in, so we return
      -1 as an invalid, sentinal value.
    */
    if (0 == entriesCount) return -1;

    size_t counter = 0;
    size_t entryIndex = 0;

    for (LineNumber currentEntry; entryIndex < entriesCount; entryIndex++) {
        currentEntry = getLineNumberArrayAt(&(chunk->lines), entryIndex);

        if (counter + currentEntry.repeats >= (size_t)offset) {
            return currentEntry.lineNumber;
        } else {
            counter += currentEntry.repeats;
        }
    }

    return getLineNumberArrayAt(&(chunk->lines), entryIndex).lineNumber;
}

void writeConstant(Chunk *chunk, Value value, int line) {
#define NTH_BYTE(number, n) ((number >> (8 * n)) & 0xFF)
    int constantIndex = addConstant(chunk, value);
    if (constantIndex >= 256) {
        writeChunk(chunk, OP_CONSTANT_LONG, line);
        writeChunk(chunk, NTH_BYTE(constantIndex, 2), line);
        writeChunk(chunk, NTH_BYTE(constantIndex, 1), line);
        writeChunk(chunk, NTH_BYTE(constantIndex, 0), line);
    } else {
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, (uint8_t)constantIndex, line);
    }
#undef NTH_BYTE
}
