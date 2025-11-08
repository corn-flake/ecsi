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

#include "common.h"
#include "line_number.h"
#include "value.h"

typedef enum OpCode {
    // Pushes a constant value onto the stack.
    // The byte after it is an index into the VM's chunk's constants array where
    // the value can be found.
    OP_CONSTANT,

    // Pushes a constant onto the stack, if the index into the constants
    // array is greater than UINT8_MAX. The next 3 bytes after this
    // instruction store the index.
    OP_CONSTANT_LONG,

    // Pushes NIL_VAL onto the stack.
    OP_NIL,

    // Pushes TRUE_VAL onto the stack.
    OP_TRUE,

    // Pushes FALSE_VAL onto the stack.
    OP_FALSE,

    // Pops one value from the stack.
    OP_POP,

    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_DEFINE_GLOBAL,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_CLOSURE,
    OP_CLOSE_UPVALUE,
    OP_RETURN,
} OpCode;

// A "chunk" of opcodes.
typedef struct {
    int count;              // Number of used bytes in code.
    int capacity;           // Number of available bytes in code.
    uint8_t *code;          // Array of instructions.
    LineNumberArray lines;  // Lines at which instructions originated from.
    ValueArray constants;   // The array that instructions that use constants
                            // index into.
} Chunk;

// Initializes the chunk pointed to by chunk.
void initChunk(Chunk *chunk);

// Adds byte to chunk's code, and records that byte originated from line of the
// source code.
void writeChunk(Chunk *chunk, uint8_t byte, int line);

/*
  Frees the memory associated with the chunk at chunk. Does not free the data
  directly at chunk, so it is safe to use on chunks that are stored on the
  stack.
*/
void freeChunk(Chunk *chunk);

/*
  Adds value to chunk's constants array, and returns the index it was placed at.
 */
int addConstant(Chunk *chunk, Value value);

/*
  Determines what line of source code the instruction at offset in chunk's code
  array originated from.
 */
int getLine(Chunk *chunk, int offset);

/*
  Adds an value to chunk's constants array, and appends an instruction
  to chunk's code to push the value onto the stack, recording that
  the added instruction originated from line of source code.
 */
void writeConstant(Chunk *chunk, Value value, int line);
