#include "object.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "table.h"
#include "value.h"
#include "vm.h"

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
      // Unreached.
      return "";
  }
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

ObjFunction *newFunction() {
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

static uint32_t hashString(const char *key, int length) {
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

ObjString *copyString(const char *chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) return interned;

  char *heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(heapChars, length, hash);
}

ObjVector *newVector() {
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

ObjSymbol *newSymbol(const char *chars, int length) {
  ObjString *string = copyString(chars, length);
  push(OBJ_VAL(string));
  ObjSymbol *symbol = ALLOCATE_OBJ(ObjSymbol, OBJ_SYMBOL);
  symbol->text = string;
  symbol->value = NIL_VAL;
  pop();  // string
  return symbol;
}

static void printFunction(ObjFunction *function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<fn %s>", function->name->chars);
}

static bool isList(ObjPair *pair) {
  while (IS_PAIR(pair->cdr)) {
    pair = AS_PAIR(pair->cdr);
  }
  return IS_NIL(pair->cdr) ? true : false;
}

static void printList(ObjPair *list) {
  while (!IS_NIL(list->cdr)) {
    printValue(list->car);
    putchar(' ');
    list = AS_PAIR(list->cdr);
  }
  printValue(list->car);
  putchar(')');
}

static void printPair(ObjPair *pair) {
  if (IS_NIL(pair->cdr)) {
    putchar('(');
    printValue(pair->car);
    putchar(')');
    return;
  }

  if (!IS_PAIR(pair->cdr)) {
    putchar('(');
    printValue(pair->car);
    printf("%s", " . ");
    printValue(pair->cdr);
    putchar(')');
    return;
  }

  if (isList(AS_PAIR(pair->cdr))) {
    putchar('(');
    printValue(pair->car);
    putchar(' ');
    printList(AS_PAIR(pair->cdr));
    return;
  }

  putchar('(');
  printValue(pair->car);
  putchar(' ');
  printPair(AS_PAIR(pair->cdr));
  putchar(')');
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

void append(ObjPair *pair, Value value) {
  while (!IS_NIL(pair->cdr)) {
    if (!IS_PAIR(pair->cdr)) {
      fprintf(stderr,
              "parser static append - pair given was not a list (not a "
              "sequence of pairs whose final pair's cdr is nil)\n");
      // We crash here because this function is only called by the parser
      // and should only be called on valid lists.
      assert(false);
    }
    pair = (ObjPair *)AS_OBJ(pair->cdr);
  }
  push(value);
  ObjPair *tail = newPair(value, NIL_VAL);
  pair->cdr = OBJ_VAL(tail);
  pop();  // value
}

size_t listLength(ObjPair *pair) {
  size_t length = 1;
  while (IS_PAIR(pair->cdr)) {
    length++;
    pair = AS_PAIR(pair->cdr);
  }
  return length;
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
