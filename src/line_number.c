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

#include "line_number.h"

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

void initLineNumberArray(LineNumberArray *array) {
    array->count = 0;
    array->capacity = 0;
    array->lineNumbers = NULL;
}

int writeNumber(LineNumberArray *array, int lineNumber) {
    if (array->count > 0 &&
        lineNumber == array->lineNumbers[array->count - 1].lineNumber) {
        array->lineNumbers[array->count - 1].repeats++;
        return array->lineNumbers[array->count - 1].lineNumber;
    }

    if (array->capacity <= array->count) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->lineNumbers = GROW_ARRAY(LineNumber, array->lineNumbers,
                                        oldCapacity, array->capacity);
    }

    LineNumber encodedNumber = {
        .lineNumber = lineNumber,
        .repeats = 1,
    };
    array->lineNumbers[array->count] = encodedNumber;
    array->count++;

    return array->lineNumbers[array->count - 1].lineNumber;
}

int *decompressLineNumberArray(LineNumberArray *array) {
    int entriesCount = numberOfEntries(array);
    size_t size = sizeof(int) * entriesCount;

    int *decompressed = malloc(size);
    if (NULL == decompressed) {
        fprintf(stderr, "decompressLineNumberArray: couldn't allocate memory.");
        exit(1);
    }

    int entryIndex = 0;
    int decompressedIndex = 0;

    while (entryIndex < array->count && decompressedIndex < entriesCount) {
        for (int i = array->lineNumbers[entryIndex].repeats; i > 0; i--) {
            decompressed[decompressedIndex] =
                array->lineNumbers[entryIndex].lineNumber;
            decompressedIndex++;
        }
        entryIndex++;
    }

    return decompressed;
}

int numberOfEntries(LineNumberArray *array) {
    int numberOfEntries = 0;
    for (int i = 0; i < array->count; i++) {
        numberOfEntries += array->lineNumbers[i].repeats;
    }
    return numberOfEntries;
}

void freeLineNumberArray(LineNumberArray *array) {
    FREE_ARRAY(LineNumber, array->lineNumbers, array->capacity);
    initLineNumberArray(array);
}
