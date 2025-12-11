/*
  Copyright 2025 Evan Cooney

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

#include <stddef.h>

#include "smart_array.h"

// An run-length encoded line number.
typedef struct {
    unsigned int lineNumber;  // The line number
    unsigned int repeats;     // The number of times the number repeats.
} LineNumber;

typedef SmartArray LineNumberArray;

// Initialize the LineNumberArray pointed to by array.
void initLineNumberArray(LineNumberArray *array);

LineNumber getLineNumberArrayAt(LineNumberArray const *array, size_t i);

/*
  Free data associated with array, and re-initialize it.
  It does not free array, so it is safe to use on stack allocated
  memory.
 */
void freeLineNumberArray(LineNumberArray *array);

/*
  Append lineNumber to array.
 */
unsigned int writeNumber(LineNumberArray *array, unsigned int lineNumber);

/*
  Return the number of entries in array.
 */
size_t numberOfEntries(LineNumberArray const *array);
