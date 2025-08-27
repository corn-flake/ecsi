#pragma once
#include "chunk.h"

void disassembleChunk(Chunk *const chunk, char const *name);
int disassembleInstruction(Chunk *const chunk, int offset);
