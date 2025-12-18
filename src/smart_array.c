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
    return 0 == getSmartArrayCapacity(smartArray);
}

bool smartArrayPopFromEnd(SmartArray *restrict smartArray, void *restrict out) {
    if (smartArrayIsEmpty(smartArray)) {
        return false;
    }

    if (NULL != out) {
        void *lastElement =
            ((char *)smartArray->data) +
            ((getSmartArrayCount(smartArray) - 1) * smartArray->elementSize);
        // void *lastElement = &(SMART_ARRAY_AT(smartArray,
        // getSmartArrayCount(smartArray) - 1));
        memcpy(out, lastElement, smartArray->elementSize);
    }

    smartArray->capacity--;
    return true;
}

void freeSmartArray(SmartArray *smartArray) {
    smartArray->reallocater(smartArray->data,
                            smartArray->capacity * smartArray->elementSize, 0);
    smartArray->count = smartArray->capacity = 0;
    smartArray->data = NULL;
}
