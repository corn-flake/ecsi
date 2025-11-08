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

// An run-length encoded line number.
typedef struct {
    int lineNumber;  // The line number
    int repeats;     // The number of times the number repeats.
} LineNumber;

typedef struct {
    int count;     // The number of used elements in lineNumbers.
    int capacity;  // The number of available elements in lineNumbers.
    LineNumber *lineNumbers;  // An array of lineNumbers.
} LineNumberArray;

// Initialize the LineNumberArray pointed to by array.
void initLineNumberArray(LineNumberArray *array);

/*
  Free data associated with array, and re-initialize it.
  It does not free array, so it is safe to use on stack allocated
  memory.
 */
void freeLineNumberArray(LineNumberArray *array);

/*
  Append lineNumber to array.
 */
int writeNumber(LineNumberArray *array, int lineNumber);

/*
  Decompress array into a normal, heap-allocated array of integers,
  and return a pointer to that array.
 */
int *decompressLineNumberArray(LineNumberArray *array);

/*
  Return the number of entries in array.
 */
int numberOfEntries(LineNumberArray *array);
