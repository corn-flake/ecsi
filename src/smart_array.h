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

#pragma once

#include <stdbool.h>
#include <stddef.h>

// A wrapper around checkedRealloc that can be used to initialize SmartArrays.
void *smartArrayCheckedRealloc(void *ptr, size_t _oldSize, size_t newSize);

typedef void *(*ReallocateFn)(void *, size_t, size_t);

typedef struct {
    size_t count, capacity, elementSize;
    ReallocateFn reallocater;
    void *data;
} SmartArray;

void initSmartArray(SmartArray *restrict smartArray, ReallocateFn reallocater,
                    size_t elementSize);

void smartArrayAppend(SmartArray *restrict smartArray,
                      void const *restrict element);

#define SMART_ARRAY_AT(smartArray, index, type) \
    ((type *)(smartArray)->data)[(index)]

size_t getSmartArrayCount(SmartArray const *smartArray);
size_t getSmartArrayCapacity(SmartArray const *smartArray);

bool smartArrayIsEmpty(SmartArray const *smartArray);
bool smartArrayPopFromEnd(SmartArray *restrict smartArray, void *restrict out);

void freeSmartArray(SmartArray *smartArray);
