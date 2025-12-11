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

void freeSmartArray(SmartArray *smartArray) {
    smartArray->reallocater(smartArray->data,
                            smartArray->capacity * smartArray->elementSize, 0);
    smartArray->count = smartArray->capacity = 0;
    smartArray->data = NULL;
}
