/*
  Copyright 2025 Evan Cooney
  Copyright 2015-2020 Robert Nystrom

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

#include "memory.h"

#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "object.h"
#include "smart_array.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>

#include "debug.h"
#endif

// Mark all values in array.
static void markArray(ValueArray *array);

// Free the object at object.
static void freeObject(Obj *object);

// Mark the garbage collector roots.
static void markRoots(void);

// Mark object and everything it references, one layer deep.
static void blackenObject(Obj *object);

// Mark all accessible objects.
static void traceReferences(void);

// Free any inaccessible objects.
static void sweep(void);

void *checkedMalloc(size_t size) {
    void *memory = malloc(size);
    if (NULL == memory) {
        DIE("Failed to allocate %zu bytes of memory.", size);
    }
    return memory;
}

void *checkedRealloc(void *ptr, size_t newSize) {
    void *result = realloc(ptr, newSize);
    if (NULL == result) {
        DIE("Failed to grow memory at %p to %zu bytes.", ptr, newSize);
    }
    return result;
}

#define GC_HEAP_GROW_FACTOR 2

void *reallocate(void *pointer, size_t oldSize, size_t newSize) {
    vm.gcState.bytesAllocated += newSize - oldSize;
    if (newSize > oldSize) {
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
    }

    if (vm.gcState.bytesAllocated > vm.gcState.nextGC) {
        collectGarbage();
    }

    if (0 == newSize) {
        free(pointer);
        return NULL;
    }

    return checkedRealloc(pointer, newSize);
}

void markObject(Obj *object) {
    if (NULL == object) return;
    if (object->isMarked) return;

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void *)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    smartArrayAppend(&(vm.gcState.grayStack), &object);
    object->isMarked = true;
}

void markValue(Value value) {
    if (IS_OBJ(value)) markObject(AS_OBJ(value));
}

void freeObjects(void) {
    Obj *object = vm.objects;
    while (object != NULL) {
        Obj *next = object->next;
#ifdef DEBUG_FREE_OBJECTS
        printf("Freeing object at %p\n", (void *)object);
        printObject(OBJ_VAL(object));
        puts("");
#endif
        freeObject(object);
        object = next;
    }

    freeSmartArray(&(vm.gcState.grayStack));
}

static void markArray(ValueArray *array) {
    for (size_t i = 0; i < getValueArrayCount(array); i++) {
        markValue(getValueArrayAt(array, i));
    }
}

static void freeObject(Obj *object) {
#ifdef DEBUG_LOG_GC
    printf("%p free type %s\n", (void *)object, objTypeToString(object->type));
#endif

    switch (object->type) {
        case OBJ_CLOSURE: {
            ObjClosure *closure = (ObjClosure *)object;
            FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalueCount);
            FREE(ObjClosure, object);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction *function = (ObjFunction *)object;
            freeChunk(&function->chunk);
            FREE(ObjFunction, object);
            break;
        }
        case OBJ_PAIR: {
            // A pair does not own its car and cdr.
            FREE(ObjPair, object);
            break;
        }
        case OBJ_STRING: {
            ObjString *string = (ObjString *)object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, object);
            break;
        }
        case OBJ_SYMBOL: {
            ObjString *text = ((ObjSymbol *)object)->text;
            FREE_ARRAY(char, text->chars, text->length + 1);
            FREE(ObjSymbol, object);
            break;
        }
        case OBJ_SYNTAX:
            FREE(ObjSyntax, object);
            break;
        case OBJ_NATIVE:
            FREE(ObjNative, object);
            break;
        case OBJ_UPVALUE:
            FREE(ObjUpvalue, object);
            break;
        case OBJ_VECTOR: {
            // A vector doesn't own its elements.
            FREE(ObjVector, object);
            break;
        }
    }
}

static void markRoots(void) {
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
        markValue(*slot);
    }

    for (int i = 0; i < vm.frameCount; i++) {
        markObject((Obj *)vm.frames[i].closure);
    }

    for (ObjUpvalue *upvalue = vm.openUpvalues; upvalue != NULL;
         upvalue = upvalue->next) {
        markObject((Obj *)upvalue);
    }

    markTable(&vm.globals);
    markCompilerRoots();
    markObject((Obj *)vm.initString);
}

static void blackenObject(Obj *object) {
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void *)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type) {
        case OBJ_CLOSURE: {
            ObjClosure *closure = (ObjClosure *)object;
            markObject((Obj *)closure->function);
            for (int i = 0; i < closure->upvalueCount; i++) {
                markObject((Obj *)closure->upvalues[i]);
            }
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction *function = (ObjFunction *)object;
            markObject((Obj *)function->name);
            markArray(&function->chunk.constants);
            break;
        }
        case OBJ_PAIR: {
            ObjPair *pair = (ObjPair *)object;
            markValue(pair->car);
            markValue(pair->cdr);
            break;
        }
        case OBJ_VECTOR:
            markArray(&(((ObjVector *)object)->array));
            break;
        case OBJ_UPVALUE:
            markValue(((ObjUpvalue *)object)->closed);
            break;
        case OBJ_SYMBOL: {
            ObjSymbol *symbol = (ObjSymbol *)object;
            markObject((Obj *)symbol->text);
            markValue(symbol->value);
            break;
        }
        case OBJ_SYNTAX:
            markValue(((ObjSyntax *)object)->value);
            break;
        case OBJ_NATIVE:
        case OBJ_STRING:
            break;
    }
}

static void traceReferences(void) {
    Obj *object = NULL;
    while (!smartArrayIsEmpty(&(vm.gcState.grayStack))) {
        smartArrayPopFromEnd(&(vm.gcState.grayStack), &object);
        blackenObject(object);
    }
}

static void sweep(void) {
    Obj *previous = NULL;
    Obj *object = vm.objects;
    while (object != NULL) {
        if (object->isMarked) {
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj *unreached = object;
            object = object->next;
            if (previous != NULL) {
                previous->next = object;
            } else {
                vm.objects = object;
            }

            freeObject(unreached);
        }
    }
}

void turnOffGarbageCollector(void) { vm.gcState.isOn = false; }

void turnOnGarbageCollector(void) { vm.gcState.isOn = true; }

void collectGarbage(void) {
#ifdef DEBUG_LOG_GC
    printStack();
    printf("\n");
    printf("-- gc begin\n");
    size_t before = vm.bytesAllocated;
#endif

    markRoots();
    traceReferences();
    tableRemoveWhite(&vm.strings);
    sweep();

    vm.gcState.nextGC = vm.gcState.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("    collected %zu bytes (from %zu to %zu) next at %zu\n",
           before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}
