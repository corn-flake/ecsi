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
