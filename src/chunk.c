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
    int writtenLine = writeNumber(&chunk->lines, line);
    assert(line == writtenLine);
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

    for (int entriesCount = numberOfEntries(&chunk->lines);
         entryIndex < entriesCount; entryIndex++) {
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
