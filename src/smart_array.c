/*
 * Copyright 2025 Evan Cooney
 *
 * This file is part of Ecsi.
 *
 * Ecsi is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Ecsi is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Ecsi. If not, see <https://www.gnu.org/licenses/>.
 */

#include "smart_array.h"

#include "memory.h"

void *smartArrayCheckedRealloc(void *ptr, size_t _oldSize, size_t newSize) {
    return checkedRealloc(ptr, newSize);
}

void initSmartArray(SmartArray *restrict smartArray, ReallocateFn reallocater,
                    size_t elementSize) {
    if (NULL == smartArray) return;
    smartArray->count = smartArray->capacity = 0;
    smartArray->elementSize = elementSize;
    smartArray->reallocater = reallocater;
    smartArray->data = NULL;
}

void smartArrayAppend(SmartArray *restrict smartArray,
                      void const *restrict element) {
    if (smartArray->capacity <= smartArray->count) {
        size_t oldCapacity = smartArray->capacity;
        smartArray->capacity = GROW_CAPACITY(oldCapacity);
        smartArray->data = smartArray->reallocater(
            smartArray->data, oldCapacity * smartArray->elementSize,
            smartArray->capacity * smartArray->elementSize);
    }

    memcpy((char *)smartArray->data +
               (smartArray->count * smartArray->elementSize),
           element, smartArray->elementSize);
    smartArray->count++;
}

size_t getSmartArrayCount(SmartArray const *smartArray) {
    return smartArray->count;
}

size_t getSmartArrayCapacity(SmartArray const *smartArray) {
    return smartArray->capacity;
}

bool smartArrayIsEmpty(SmartArray const *smartArray) {
    return 0 == getSmartArrayCount(smartArray);
}

bool smartArrayPopFromEnd(SmartArray *restrict smartArray, void *restrict out) {
    if (smartArrayIsEmpty(smartArray)) {
        return false;
    }

    if (NULL != out) {
        void *lastElement = &(SMART_ARRAY_AT(
            smartArray,
            (getSmartArrayCount(smartArray) - 1) * smartArray->elementSize,
            char));
        memcpy(out, lastElement, smartArray->elementSize);
    }

    smartArray->count--;
    return true;
}

void freeSmartArray(SmartArray *smartArray) {
    smartArray->reallocater(smartArray->data,
                            smartArray->capacity * smartArray->elementSize, 0);
    smartArray->count = smartArray->capacity = 0;
    smartArray->data = NULL;
}
