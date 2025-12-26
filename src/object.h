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

#pragma once

#include <stdint.h>

typedef struct ObjString ObjSymbol;

#include "chunk.h"
#include "scanner.h"
#include "value.h"

// Retrieve the object type value. Value must be an object.
#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_PAIR(value) isObjType(value, OBJ_PAIR)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_SYMBOL(value) isObjType(value, OBJ_SYMBOL)
#define IS_SYNTAX(value) isObjType(value, OBJ_SYNTAX)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_UPVALUE(value) isObjType(value, OBJ_UPVALUE)
#define IS_VECTOR(value) isObjType(value, OBJ_VECTOR)

#define AS_CLOSURE(value) ((ObjClosure *)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction *)AS_OBJ(value))
#define AS_PAIR(value) ((ObjPair *)AS_OBJ(value))
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)
#define AS_SYMBOL(value) ((ObjSymbol *)AS_OBJ(value))
#define AS_SYNTAX(value) ((ObjSyntax *)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative *)AS_OBJ(value))->function)
#define AS_UPVALUE(value) ((ObjUpvalue *)AS_OBJ(value))
#define AS_VECTOR(value) ((ObjVector *)AS_OBJ(value))

/*
  Create a new pair with car as its car and cdr as its cdr, and
  treat it as a value.
*/
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

// The type of object something is.
typedef enum {
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_PAIR,
    OBJ_STRING,
    OBJ_SYMBOL,
    OBJ_SYNTAX,
    OBJ_NATIVE,
    OBJ_UPVALUE,
    OBJ_VECTOR,
} ObjType;

// Convert a ObjType to a string representation.
char const *objTypeToString(ObjType type);

// Scheme object metadata.
struct Obj {
    ObjType type;      // Type of object
    bool isMarked;     // True if accessible by other objects.
    struct Obj *next;  // Next object in VM's objects list.
};

// A Scheme symbol.
struct ObjSymbol {
    Obj obj;      // Metadata
    int length;   // Length of the text
    char *chars;  // Null terminated text string

    // We store the symbols's hash because symbols are interned.
    uint32_t hash;  // The hash of the string, calculated with hashString.
};

// A real Scheme string which has a fixed length
typedef struct {
    Obj obj;
    size_t length;  // Length of the string
    char *chars;    // Null terminated data
} ObjFixedString;

typedef struct {
    Obj obj;
    Value value;
    SourceLocation location;
} ObjSyntax;

typedef struct ObjUpvalue {
    Obj obj;
    Value *location;
    Value closed;
    struct ObjUpvalue *next;
} ObjUpvalue;

// A Scheme function
typedef struct {
    Obj obj;    // Metadata
    int arity;  // Number of arguments
    int upvalueCount;
    Chunk chunk;      // Function code
    ObjSymbol *name;  // Function name
} ObjFunction;

// A Scheme closure.
typedef struct {
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int upvalueCount;
} ObjClosure;

// An alias for pointers to Scheme functions implemented in C.
typedef Value (*NativeFn)(int argCount, Value *args);

// A Scheme function which is implemented in C.
typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

// A Scheme pair.
typedef struct {
    Obj obj;
    Value car;
    Value cdr;
} ObjPair;

typedef struct {
    Obj obj;
    ValueArray array;
} ObjVector;

/*
struct ObjSymbol {
    Obj obj;
    ObjString *text;
};
*/

/*
  Determine whether value is an object of type type.

  We define this as an inline function instead of a macro because
  its body uses 'value' twice. Because this is a function, that
  means value only gets evaluated once. If it was a macro, then the
  expression passed in as 'value' would get evaluated more than once,
  which is not what we want if that expression has side-effects.
*/
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

// Return a heap-allocated string representation of value.
char *objectToString(Value value);

// Create a new closure whose underlying function is function.
ObjClosure *newClosure(ObjFunction *function);

// Create a new Scheme function.
ObjFunction *newFunction(void);

/*
  Create a new pair whose car is car and whose cdr is cdr. Return the
  result as an ObjPair.
 */
ObjPair *newPair(Value car, Value cdr);

// Create a new native function object from the function pointer function.
ObjNative *newNative(NativeFn function);

ObjString *takeString(char *chars, int length);

// Copy length chars into a newly allocated Scheme string.
ObjString *copyString(char const *chars, int length);

// Create a new vector.
ObjVector *newVector(void);

// Append value to vector.
void vectorAppend(ObjVector *vector, Value value);

ObjUpvalue *newUpvalue(Value *slot);

// Create a new symbol, length long, with chars as its text.
ObjSymbol *newSymbol(char const *chars, int length);

// Create a new syntax object with value as its value at location.
ObjSyntax *newSyntax(Value value, SourceLocation location);

// Print the text representation of value, interpreted as an object.
void printObject(Value value);

void appendElement(ObjPair *pair, Value value);

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
