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
#include "smart_array.h"

static inline void increaseRepeatsOfEntryAt(LineNumberArray *array, size_t i);

void initLineNumberArray(LineNumberArray *array) {
    initSmartArray(array, reallocate, sizeof(LineNumber));
}

LineNumber getLineNumberArrayAt(LineNumberArray const *array, size_t i) {
    return SMART_ARRAY_AT(array, i, LineNumber);
}

unsigned int writeNumber(LineNumberArray *array, unsigned int lineNumber) {
    if (!smartArrayIsEmpty(array) &&
        getLineNumberArrayAt(array, getSmartArrayCount(array) - 1).lineNumber ==
            lineNumber) {
        increaseRepeatsOfEntryAt(array, getSmartArrayCount(array) - 1);
        return getLineNumberArrayAt(array, getSmartArrayCount(array) - 1)
            .lineNumber;
    }

    LineNumber encodedNumber = {
        .lineNumber = lineNumber,
        .repeats = 1,
    };

    smartArrayAppend(array, &encodedNumber);
    return getLineNumberArrayAt(array, getSmartArrayCount(array) - 1)
        .lineNumber;
}

static inline void increaseRepeatsOfEntryAt(LineNumberArray *array, size_t i) {
    SMART_ARRAY_AT(array, i, LineNumber).repeats++;
}

size_t numberOfEntries(LineNumberArray const *array) {
    size_t numberOfEntries = 0;
    for (size_t i = 0; i < getSmartArrayCount(array); i++) {
        numberOfEntries += getLineNumberArrayAt(array, i).repeats;
    }
    return numberOfEntries;
}

void freeLineNumberArray(LineNumberArray *array) { freeSmartArray(array); }
