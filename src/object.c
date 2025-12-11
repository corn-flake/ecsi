
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

#include "object.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "memory.h"
#include "table.h"
#include "value.h"
#include "vm.h"

typedef struct {
    size_t count;
    size_t capacity;
    char *string;
} GrowableString;

/*
  There is no growableStringFree function because growableStrings
  are not currently intended to be freed. There are only used
  internally in the objectToString functions, which are only for
  interactive debugging.
 */
static void initGrowableString(GrowableString *gs);
static void growableStringAppendChar(GrowableString *gs, char c);
static void growableStringAppendString(GrowableString *gs, char const *s);

static char *objStringToString(ObjString const *string);
static char *objPairToString(ObjPair const *pair);
static void objPairToStringInGrowableString(ObjPair const *pair,
                                            GrowableString *gs);
static char *listToString(ObjPair const *list);
static char *objClosureToString(ObjClosure const *closure);
static char *objFunctionToString(ObjFunction const *function);
static char *objSymbolToString(ObjSymbol const *symbol);
static char *objVectorToString(ObjVector const *vector);

static Obj *allocateObject(size_t size, ObjType type);
static ObjString *allocateString(char *chars, int length, uint32_t hash);
static uint32_t hashString(char const *key, int length);
static void printFunction(ObjFunction const *function);
static bool isList(ObjPair const *pair);
static void printList(ObjPair const *pair);
static void printPair(ObjPair const *pair);
static bool objStringEqualToString(ObjString const *string, char const *chars);

static void initGrowableString(GrowableString *gs) {
    gs->capacity = gs->count = 0;
    gs->string = checkedMalloc(1);
    *(gs->string) = '\0';
}

static void growableStringAppendChar(GrowableString *gs, char c) {
    if (gs->capacity < gs->count + 1) {
        size_t oldCapacity = gs->capacity;
        gs->capacity = GROW_CAPACITY(oldCapacity);
        char *newString = checkedMalloc(gs->capacity);
        memcpy(newString, gs->string, gs->count);
        gs->string = newString;
    }

    gs->string[gs->count] = c;
    gs->string[gs->count + 1] = '\0';
    gs->count++;
}

static void growableStringAppendString(GrowableString *gs, const char *s) {
    for (; *s; s++) {
        growableStringAppendChar(gs, *s);
    }
}

const char *objTypeToString(ObjType type) {
    switch (type) {
        case OBJ_CLOSURE:
            return "OBJ_CLOSURE";
        case OBJ_STRING:
            return "OBJ_STRING";
        case OBJ_NATIVE:
            return "OBJ_NATIVE";
        case OBJ_FUNCTION:
            return "OBJ_FUNCTION";
        case OBJ_UPVALUE:
            return "OBJ_UPVALUE";
        case OBJ_SYMBOL:
            return "OBJ_SYMBOL";
        case OBJ_PAIR:
            return "OBJ_PAIR";
        case OBJ_VECTOR:
            return "OBJ_VECTOR";
        default:
            UNREACHABLE();
    }
}

char *objectToString(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_CLOSURE:
            return objClosureToString(AS_CLOSURE(value));
        case OBJ_FUNCTION:
            return objFunctionToString(AS_FUNCTION(value));
        case OBJ_PAIR:
            return objPairToString(AS_PAIR(value));
        case OBJ_STRING:
            return objStringToString(AS_STRING(value));
        case OBJ_SYMBOL:
            return objSymbolToString(AS_SYMBOL(value));
            // We heap-allocate OBJ_NATIVE and OBJ_UPVALUE because other
            // functions expect them to be heap-allocated.
        case OBJ_NATIVE: {
            size_t const NATIVE_FN_LEN = 11;
            char *nativeFnString = checkedMalloc(NATIVE_FN_LEN + 1);
            memcpy(nativeFnString, "<native fn>", NATIVE_FN_LEN + 1);
            return nativeFnString;
        }
        case OBJ_UPVALUE: {
            size_t const UPVALUE_LEN = 7;
            char *upvalueString = checkedMalloc(UPVALUE_LEN + 1);
            memcpy(upvalueString, "upvalue", UPVALUE_LEN + 1);
            return upvalueString;
        }
        case OBJ_VECTOR:
            return objVectorToString(AS_VECTOR(value));
        default:
            // Unreached
            return NULL;
    }
}

static char *objStringToString(ObjString const *string) {
    size_t bufferSize = string->length + 2;  // 2 for double quotes, 1 for null.
    char *buffer = ALLOCATE(char, bufferSize);
    buffer[0] = '"';
    memcpy(buffer + 1, string->chars, string->length);
    buffer[bufferSize - 2] = '"';
    buffer[bufferSize - 1] = '\0';
    return buffer;
}

static char *objPairToString(ObjPair const *pair) {
    GrowableString gs;
    initGrowableString(&gs);
    objPairToStringInGrowableString(pair, &gs);
    return gs.string;
}

static void objPairToStringInGrowableString(ObjPair const *pair,
                                            GrowableString *gs) {
    if (IS_NIL(pair->cdr)) {
        growableStringAppendChar(gs, '(');
        char *carString = valueToString(pair->car);
        growableStringAppendString(gs, carString);
        free(carString);
        growableStringAppendChar(gs, ')');
        return;
    }

    if (!IS_PAIR(pair->cdr)) {
        growableStringAppendChar(gs, '(');

        char *carString = valueToString(pair->car);
        growableStringAppendString(gs, carString);
        free(carString);

        growableStringAppendString(gs, " . ");

        char *cdrString = valueToString(pair->cdr);
        growableStringAppendString(gs, cdrString);
        free(cdrString);

        growableStringAppendChar(gs, ')');
    }

    if (isList(AS_PAIR(pair->cdr))) {
        char *carString = valueToString(pair->car);
        growableStringAppendString(gs, carString);
        free(carString);

        growableStringAppendChar(gs, ' ');

        char *listString = listToString(AS_PAIR(pair->cdr));
        growableStringAppendString(gs, listString);
        free(listString);
    }

    char *carString = valueToString(pair->car);
    growableStringAppendString(gs, carString);
    free(carString);

    growableStringAppendChar(gs, ' ');

    char *cdrString = objPairToString(AS_PAIR(pair->cdr));
    growableStringAppendString(gs, cdrString);
    free(cdrString);

    growableStringAppendChar(gs, ')');
}

static char *listToString(ObjPair const *list) {
    GrowableString gs;
    initGrowableString(&gs);

    while (!IS_NIL(list->cdr)) {
        char *carString = valueToString(list->car);
        growableStringAppendString(&gs, carString);
        free(carString);
        growableStringAppendChar(&gs, ' ');
        list = AS_PAIR(list->cdr);
    }

    char *finalCarString = valueToString(list->car);
    growableStringAppendString(&gs, finalCarString);
    free(finalCarString);
    growableStringAppendChar(&gs, ')');

    return gs.string;
}

static char *objClosureToString(ObjClosure const *closure) {
    return objFunctionToString(closure->function);
}

static char *objFunctionToString(ObjFunction const *function) {
    if (NULL == function->name) {
        size_t scriptStringLength = 7;
        char *scriptString = checkedMalloc(scriptStringLength + 1);
        memcpy(scriptString, "script", scriptStringLength);
        scriptString[scriptStringLength] = '\0';
        return scriptString;
    }

    size_t bufferSize = function->name->length + 3;  // <fn > + null
    char *buffer = checkedMalloc(bufferSize);
    memcpy(buffer, "<fn", 3);
    memcpy(buffer + 3, function->name->chars, function->name->length);
    buffer[function->name->length + 3] = '>';
    buffer[bufferSize - 1] = '\0';
    return buffer;
}

static char *objSymbolToString(ObjSymbol const *symbol) {
    size_t bufferSize = symbol->text->length + 2;  // single quote + null
    char *buffer = ALLOCATE(char, bufferSize);
    buffer[0] = '\'';
    memcpy(buffer + 1, symbol->text->chars, symbol->text->length);
    buffer[bufferSize - 1] = '\0';
    return buffer;
}

static char *objVectorToString(ObjVector const *vector) {
    GrowableString vecString;
    initGrowableString(&vecString);
    growableStringAppendString(&vecString, "#(");

    char *elem = valueToString(getValueArrayAt(&(vector->array), 0));
    growableStringAppendString(&vecString, elem);
    free(elem);

    for (size_t i = 1; i < vector->array.count; i++) {
        growableStringAppendChar(&vecString, ' ');
        elem = valueToString(getValueArrayAt(&(vector->array), i));
        growableStringAppendString(&vecString, elem);
        free(elem);
    }

    growableStringAppendChar(&vecString, ')');
    return vecString.string;
}

#define ALLOCATE_OBJ(type, objectType) \
    (type *)allocateObject(sizeof(type), objectType)

static Obj *allocateObject(size_t size, ObjType type) {
    Obj *object = (Obj *)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %s\n", (void *)object, size,
           objTypeToString(type));
#endif

    return object;
}

ObjClosure *newClosure(ObjFunction *function) {
    ObjUpvalue **upvalues = ALLOCATE(ObjUpvalue *, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }
    ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjFunction *newFunction(void) {
    ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjPair *newPair(Value car, Value cdr) {
    ObjPair *pair = ALLOCATE_OBJ(ObjPair, OBJ_PAIR);
    pair->car = car;
    pair->cdr = cdr;
    return pair;
}

ObjNative *newNative(NativeFn function) {
    ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

static ObjString *allocateString(char *chars, int length, uint32_t hash) {
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, NIL_VAL);
    pop();

    return string;
}

static uint32_t hashString(char const *key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjString *takeString(char *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm.strings, chars, length, hash);

    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjString *copyString(char const *chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) return interned;

    char *heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

ObjVector *newVector(void) {
    ObjVector *vector = ALLOCATE_OBJ(ObjVector, OBJ_VECTOR);
    initValueArray(&vector->array);
    return vector;
}

void vectorAppend(ObjVector *vector, Value value) {
    writeValueArray(&vector->array, value);
}

ObjUpvalue *newUpvalue(Value *slot) {
    ObjUpvalue *upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NIL_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

ObjSymbol *newSymbol(char const *chars, int length) {
    ObjString *string = copyString(chars, length);
    push(OBJ_VAL(string));
    ObjSymbol *symbol = ALLOCATE_OBJ(ObjSymbol, OBJ_SYMBOL);
    symbol->text = string;
    symbol->value = NIL_VAL;
    pop();  // string
    return symbol;
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_CLOSURE:
            printFunction(AS_CLOSURE(value)->function);
            break;
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
        case OBJ_STRING:
            printf("\"%s\"", AS_CSTRING(value));
            break;
        case OBJ_SYMBOL: {
            ObjString *text = AS_SYMBOL(value)->text;
            char *chars = text->chars;
            printf("'%s", chars);
            break;
        }
        case OBJ_PAIR: {
            printPair(AS_PAIR(value));
            break;
        }
        case OBJ_NATIVE:
            printf("<native fn>");
            break;
        case OBJ_UPVALUE:
            printf("upvalue");
            break;
        case OBJ_VECTOR:
            printf("%s", "#(");
            printValueArray(&AS_VECTOR(value)->array);
            putchar(')');
            break;
    }
}

static void printFunction(ObjFunction const *function) {
    if (NULL == function->name) {
        printf("<script>");
        return;
    }
    printf("<fn %s>", function->name->chars);
}

static void printPair(ObjPair const *pair) {
    putchar('(');

    if (IS_NIL(pair->cdr)) {
        printValue(pair->car);
        putchar(')');
        return;
    }

    if (!IS_PAIR(pair->cdr)) {
        printValue(pair->car);
        printf("%s", " . ");
        printValue(pair->cdr);
        putchar(')');
        return;
    }

    if (isList(AS_PAIR(pair->cdr))) {
        printValue(pair->car);
        putchar(' ');
        printList(AS_PAIR(pair->cdr));
        return;
    }

    printValue(pair->car);
    putchar(' ');
    printPair(AS_PAIR(pair->cdr));
    putchar(')');
}

static bool isList(ObjPair const *pair) {
    while (IS_PAIR(pair->cdr)) {
        pair = AS_PAIR(pair->cdr);
    }
    return IS_NIL(pair->cdr);
}

static void printList(ObjPair const *list) {
    while (!IS_NIL(list->cdr)) {
        printValue(list->car);
        putchar(' ');
        list = AS_PAIR(list->cdr);
    }
    printValue(list->car);
    putchar(')');
}

void appendElement(ObjPair *pair, Value value) {
    ObjPair *originalPair = pair;
    // Nothing to do
    if (IS_NIL(value)) return;

    while (!IS_NIL(pair->cdr)) {
        if (!IS_PAIR(pair->cdr)) {
            // We crash here because this function is only called by the parser
            // and should only be called on valid lists.
            DIE("%s", "pair given was not a list.");
        }
        pair = AS_PAIR(pair->cdr);
    }

    push(value);
    push(OBJ_VAL(originalPair));

    ObjPair *tail = newPair(value, NIL_VAL);
    pair->cdr = OBJ_VAL(tail);

    pop();  // originalPair
    pop();  // value
}

void guardedAppend(Value list, Value elem) {
    if (IS_OBJ(elem)) push(elem);
    appendElement(AS_PAIR(list), elem);
    if (IS_OBJ(elem)) pop();  // elem
}

size_t listLength(ObjPair const *pair) {
    if (IS_NIL(pair->cdr)) return 1;
    size_t length = 0;
    while (!IS_NIL(pair->cdr) && IS_PAIR(pair->cdr)) {
        length++;
        pair = AS_PAIR(pair->cdr);
    }
    return length;
}

Value guardedCons(Value car, Value cdr) {
    if (IS_OBJ(car)) push(car);
    if (IS_OBJ(cdr)) push(cdr);
    Value pair = CONS(car, cdr);
    if (IS_OBJ(car)) pop();
    if (IS_OBJ(cdr)) pop();
    return pair;
}

ObjPair *finalPair(ObjPair *list) {
    if (!IS_PAIR(list->cdr)) {
        return list;
    }

    while (IS_PAIR(list->cdr)) {
        list = AS_PAIR(list->cdr);
    }

    return list;
}

bool textOfSymbolEqualToString(ObjSymbol const *symbol, const char *string) {
    return objStringEqualToString(symbol->text, string);
}

static bool objStringEqualToString(ObjString const *string, char const *chars) {
    if (strlen(chars) != (size_t)string->length) return false;
    return !memcmp(chars, string->chars, string->length);
}
