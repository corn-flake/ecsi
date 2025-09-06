#pragma once

#include <stdint.h>

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_PAIR(value) isObjType(value, OBJ_PAIR)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_SYMBOL(value) isObjType(value, OBJ_SYMBOL)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_UPVALUE(value) isObjType(value, OBJ_UPVALUE)
#define IS_VECTOR(value) isObjType(value, OBJ_VECTOR)

#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_PAIR(value) ((ObjPair *)AS_OBJ(value))
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)
#define AS_SYMBOL(value) ((ObjSymbol *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_UPVALUE(value) ((ObjUpvalue *)AS_OBJ(value))
#define AS_VECTOR(value) ((ObjVector *)AS_OBJ(value))

#define CONS(car, cdr) OBJ_VAL(newPair(car, cdr))

#define CAR(value) (AS_PAIR(value)->car)
#define CDR(value) (AS_PAIR(value)->cdr)

#define CAAR(value) (CAR(CAR(value)))
#define CADR(value) (CAR(CDR(value)))
#define CDAR(value) (CDR(CAR(value)))
#define CDDR(value) (CDR(CDR(value)))

#define CAAAR(value) (CAR(CAAR(value)))
#define CAADR(value) (CAR(CADR(value)))
#define CADAR(value) (CAR(CDAR(value)))
#define CADDR(value) (CAR(CDDR(value)))
#define CDAAR(value) (CDR(CAAR(value)))
#define CDADR(value) (CDR(CADR(value)))
#define CDDAR(value) (CDR(CDAR(value)))
#define CDDDR(value) (CDR(CDDR(value)))

#define SET_CAR(value, new) (AS_PAIR(value)->car = new)
#define SET_CDR(value, new) (AS_PAIR(value)->cdr = new)

#define SET_CAAR(value, new) (CAR(CAR(value)) = new)
#define SET_CADR(value, new) (CAR(CDR(value)) = new)
#define SET_CDAR(value, new) (CDR(CAR(value)) = new)
#define SET_CDDR(value, new) (CDR(CDR(value)) = new)

#define SET_CAAAR(value, new) (CAR(CAAR(value)) = new)
#define SET_CAADR(value, new) (CAR(CADR(value)) = new)
#define SET_CADAR(value, new) (CAR(CDAR(value)) = new)
#define SET_CADDR(value, new) (CAR(CDDR(value)) = new)
#define SET_CDAAR(value, new) (CDR(CAAR(value)) = new)
#define SET_CDADR(value, new) (CDR(CADR(value)) = new)
#define SET_CDDAR(value, new) (CDR(CDAR(value)) = new)
#define SET_CDDDR(value, new) (CDR(CDDR(value)) = new)

typedef enum {
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_PAIR,
    OBJ_STRING,
    OBJ_SYMBOL,
    OBJ_NATIVE,
    OBJ_UPVALUE,
    OBJ_VECTOR,
} ObjType;

char const *objTypeToString(ObjType type);

struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj *next;
};

struct ObjString {
    Obj obj;
    int length;
    char *chars;
    // We store the string's hash because strings in Lox are
    // immutable and it improves performance to not re-hash the string
    // every time we look for a key in the hash table
    uint32_t hash;
};

typedef struct ObjUpvalue {
    Obj obj;
    Value *location;
    Value closed;
    struct ObjUpvalue *next;
} ObjUpvalue;

typedef struct {
    Obj obj;
    int arity;
    int upvalueCount;
    Chunk chunk;
    ObjString *name;
} ObjFunction;

typedef struct {
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int upvalueCount;
} ObjClosure;

typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

typedef struct {
    Obj obj;
    Value car;
    Value cdr;
} ObjPair;

typedef struct {
    Obj *obj;
    ValueArray array;
} ObjVector;

typedef struct {
    Obj obj;
    ObjString *text;
    Value value;
} ObjSymbol;

/*
  We define this as an inline function instead of a macro because
  its body uses 'value' twice. Because this is a function, that
  means value only gets evaluated once. If it was a macro, then the
  expression passed in as 'value' would get evaluated more than once,
  which is not what we want if that expression has side-effects.
*/
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

char *objectToString(Value value);

ObjClosure *newClosure(ObjFunction *function);
ObjFunction *newFunction();
ObjPair *newPair(Value car, Value cdr);
ObjNative *newNative(NativeFn function);
ObjString *takeString(char *chars, int length);
ObjString *copyString(char const *chars, int length);
ObjVector *newVector();
void vectorAppend(ObjVector *vector, Value value);
ObjUpvalue *newUpvalue(Value *slot);
ObjSymbol *newSymbol(char const *chars, int length);

void printObject(Value value);

// Utility functions

void append(ObjPair *pair, Value value);

// Appends elem to list, but pushes elem onto the stack first.
// It pops elem when done.
// It triggers the GC.
void guardedAppend(Value list, Value elem);
size_t listLength(ObjPair const *pair);
ObjPair *finalPair(ObjPair *pair);
// CONS, but protects its arguments from the garbage collector.
// It triggers the GC.
Value guardedCons(Value car, Value cdr);
bool textOfSymbolEqualToString(ObjSymbol const *symbol, char const *string);
