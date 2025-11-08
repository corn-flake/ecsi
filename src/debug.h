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
#include "chunk.h"

/*
  Prints the disassembly of chunk, titled as name.
 */
void disassembleChunk(Chunk *const chunk, char const *name);

/*
  Prints the disassembly of the instruction at offset in chunk,
  and returns the index of the next instruction.
 */
int disassembleInstruction(Chunk *const chunk, int offset);
