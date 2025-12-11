

#pragma once

#include <stddef.h>

#include "common.h"
#include "object.h"

/*
  Allocate memory for count elements of type, using the Ecsi
  memory allocation system.
*/
#define ALLOCATE(type, count) \
    (type *)reallocate(NULL, 0, sizeof(type) * (count))

// Free a pointer to type, in Ecsi's memory allocation system.
#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

/*
  Calculate what the new capacity of an array should be, based on
  capacity.
*/
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

/*
  Change the number of elements allocated to the array of type at pointer from
  oldCount to newCount.
 */
#define GROW_ARRAY(type, pointer, oldCount, newCount)      \
    (type *)reallocate(pointer, sizeof(type) * (oldCount), \
                       sizeof(type) * (newCount))

/*
  Free the array of type at pointer, whose current number of elements is
  oldCount.
 */
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

/*
  Allocates size bytes of memory on the heap. In the case of allocation
  failure, it prints an appropriate error message and exits the program.
  The memory it allocates is *not* tracked by the garbage collector, so
  should not be used for Scheme objects. However, the garbage collector
  does use this function internally.
 */
void *checkedMalloc(size_t size);

/*
  Similar to checkedMalloc, it resizes the memory at ptr to newSize bytes.
  The memory it allocates is also *not* tracked by the garbage collector.
 */
void *checkedRealloc(void *ptr, size_t newSize);

/*
  Resizes the memory at pointer from oldSize, to newSize. The memory
  it allocates is tracked by the garbage collector.
 */
void *reallocate(void *pointer, size_t oldSize, size_t newSize);

// Mark object as accessible, and to be spared from the garbage collector.
// Does not mark anything object references.
void markObject(Obj *object);

// Mark value as accessible, and to be spared by the garbage collector.
void markValue(Value value);

// Run the garbage collector.
void collectGarbage(void);

// Free all unreachable objects.
void freeObjects(void);
