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

#include "debug.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "object.h"
#include "value.h"

/*
  Prints name with a new line after it and returns the index of the next
  instruction. Used for instructions that take no operands.
 */
static int simpleInstruction(char const *name, int offset);

/*
  Prints name left-aligned in a 16 character field, and operand right-aligned
  in a 4 character field, then a single space character.
 */
static void printInstructionNameAndOperand(char const *name, uint32_t operand);

/*
  Prints the 2-byte instruction at offset in chunk, labeling it as name, and
  return the offset of the next instruction.
 */
static int byteInstruction(char const *name, Chunk const *chunk, int offset);

/*
  Prints a jump instruction, where it jumps to, and returns the offset of the
  next instruction.
 */
static int jumpInstruction(char const *name, int sign, Chunk const *chunk,
                           int offset);

/*
  Prints an instruction that refers to a constant with an 1 byte operand, and
  return the offset of the next operand.
 */
static int constantInstruction(char const *name, Chunk const *chunk,
                               int offset);

/*
  Prints an instruction with op code OP_CONSTANT_LONG, and returns
  the offset of the next instruction.
 */
static int constantLongInstruction(char const *name, Chunk const *chunk,
                                   int offset);

void disassembleChunk(Chunk *const chunk, char const *name) {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

int disassembleInstruction(Chunk *const chunk, int offset) {
    printf("%04d ", offset);

    printf("%4d ", getLine(chunk, offset));

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_CONSTANT_LONG:
            return constantLongInstruction("OP_CONSTANT_LONG", chunk, offset);
        case OP_NIL:
            return simpleInstruction("OP_NIL", offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        case OP_POP:
            return simpleInstruction("OP_POP", offset);
        case OP_GET_LOCAL:
            return byteInstruction("OP_GET_LOCAL", chunk, offset);
        case OP_SET_LOCAL:
            return byteInstruction("OP_SET_LOCAL", chunk, offset);
        case OP_GET_GLOBAL:
            return constantInstruction("OP_GET_GLOBAL", chunk, offset);
        case OP_DEFINE_GLOBAL:
            return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
        case OP_SET_GLOBAL:
            return constantInstruction("OP_SET_GLOBAL", chunk, offset);
        case OP_GET_UPVALUE:
            return byteInstruction("OP_GET_UPVALUE", chunk, offset);
        case OP_SET_UPVALUE:
            return byteInstruction("OP_SET_UPVALUE", chunk, offset);
        case OP_JUMP:
            return jumpInstruction("OP_JUMP", 1, chunk, offset);
        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
        case OP_LOOP:
            return jumpInstruction("OP_LOOP", -1, chunk, offset);
        case OP_CALL:
            return byteInstruction("OP_CALL", chunk, offset);
        case OP_CLOSURE: {
            offset++;
            uint8_t constant = chunk->code[offset++];
            printInstructionNameAndOperand("OP_CLOSURE", constant);
            printValue(chunk->constants.values[constant]);
            puts("");

            ObjFunction *function =
                AS_FUNCTION(chunk->constants.values[constant]);
            for (int j = 0; j < function->upvalueCount; j++) {
                int isLocal = chunk->code[offset++];
                int index = chunk->code[offset++];
                printf("%04d      |                     %s %d\n", offset - 2,
                       isLocal ? "local" : "upvalue", index);
            }
            return offset;
        }
        case OP_CLOSE_UPVALUE:
            return simpleInstruction("OP_CLOSE_UPVALUE", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

static int byteInstruction(const char *name, Chunk const *chunk, int offset) {
    uint8_t slot = chunk->code[offset + 1];
    printInstructionNameAndOperand(name, slot);
    puts("");
    return offset + 2;
}

static int jumpInstruction(const char *name, int sign, Chunk const *chunk,
                           int offset) {
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    printInstructionNameAndOperand(name, offset);
    printf("-> %d\n", offset + 3 + sign * jump);
    return offset + 3;
}

static int constantInstruction(const char *name, Chunk const *chunk,
                               int offset) {
    uint8_t constant = chunk->code[offset + 1];
    printInstructionNameAndOperand(name, constant);
    putchar('\'');
    printValue(chunk->constants.values[constant]);
    puts("'");
    return offset + 2;
}

static int constantLongInstruction(char const *name, Chunk const *chunk,
                                   int offset) {
    /*
      This shifting and or-ing code is necessary instead of something
      like casting chunk->code to a uint32_t * and derefencing, then masking,
      or using memcpy because I think my computer is flipping the bytes somehow.
      When I used casting or memcpy, the bytes ended up in flipped order.
    */

    uint32_t constantIndex = (uint32_t)(chunk->code[offset + 1] << 8);
    constantIndex |= chunk->code[offset + 2];
    constantIndex <<= 8;
    constantIndex |= chunk->code[offset + 3];

    uint32_t constantIndexMemcpy = 0;
    memcpy(&constantIndexMemcpy, chunk->code + offset + 1, 3);

    printf("\nmemcpy: 0x%08X\nshifting: 0x%08X\n", constantIndexMemcpy,
           constantIndex);

    Value constantValue = chunk->constants.values[constantIndex];
    printInstructionNameAndOperand(name, constantIndex);
    putchar('\'');
    printValue(constantValue);
    puts("'");
    return offset + 4;
}

static void printInstructionNameAndOperand(char const *name, uint32_t operand) {
    printf("%-16s %4u ", name, operand);
}

static int simpleInstruction(const char *name, int offset) {
    puts(name);
    return offset + 1;
}
