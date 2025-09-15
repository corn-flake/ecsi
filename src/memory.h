

#pragma once

#include <stddef.h>

#include "common.h"
#include "object.h"

#define ALLOCATE(type, count) \
    (type *)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount)      \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), \
                       sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

// checkedMalloc should only be used for memory that should not be GC'd.
// For example, valueToString uses this because it's for debugging. It doesn't
// create strings that are used as Lisp objects.
void *checkedMalloc(size_t size);

// Although checkedRealloc is used by the GC, if you're calling it directly,
// the memory it returns IS NOT GC'd.
void *checkedRealloc(void *ptr, size_t newSize);

void *reallocate(void *pointer, size_t oldSize, size_t newSize);
void markObject(Obj *object);
void markValue(Value value);
void collectGarbage();
void freeObjects();
