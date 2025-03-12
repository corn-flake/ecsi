#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"
#include "memory.h"

VM vm;

static void resetStack() {
     FREE_ARRAY(Value, vm.stack, vm.stackCapacity);
     vm.stackCapacity = 0;
     vm.stackTop = vm.stack;
}

void initVM() {
     vm.stack = NULL;
     vm.stackCapacity = 0;
     vm.stackTop = vm.stack;
}

void freeVM() {
     resetStack();
}

void push(Value value) {
     if (vm.stackTop >= vm.stack + vm.stackCapacity) {
	  int oldCapacity = vm.stackCapacity;
	  int stackHeight = vm.stackTop - vm.stack;
	  vm.stackCapacity = GROW_CAPACITY(oldCapacity);
	  vm.stack = GROW_ARRAY(Value, vm.stack, oldCapacity, vm.stackCapacity);
	  vm.stackTop = vm.stack + stackHeight;
     }
     *vm.stackTop = value;
     vm.stackTop++;
}

Value pop() {
     // We shrink the stack if more than half of it is empty so that
     // we don't waste a bunch of memory if the program requires a big stack,
     // then totally empties it and never fills it again.
     if ((vm.stackTop - vm.stack) * 2 <= vm.stackCapacity) {
	  int oldCapacity = vm.stackCapacity;
	  vm.stackCapacity /= 2;
	  vm.stack = GROW_ARRAY(Value, vm.stack, oldCapacity, vm.stackCapacity);
     }
     vm.stackTop--;
     return *vm.stackTop;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)				\
     do {					\
	  double b = pop();			\
	  double a = pop();			\
	  push(a op b);				\
     } while(false)

     for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
	  printf("       ");
	  for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
	       printf("[ ");
	       printValue(*slot);
	       printf(" ]");
	  }
	  printf("\n");
	  disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
	  uint8_t instruction;
	  switch (instruction = READ_BYTE()) {
	  case OP_CONSTANT: {
	       Value constant = READ_CONSTANT();
	       push(constant);
	       break;
	  }
	  case OP_CONSTANT_LONG: {
	       int constantIndex = READ_BYTE();
	       constantIndex = constantIndex << 8;
	       constantIndex += READ_BYTE();
	       constantIndex = constantIndex << 8;
	       constantIndex += READ_BYTE();
	       Value constantValue = vm.chunk->constants.values[constantIndex];
	       push(constantValue);
	       break;
	  }
	  case OP_ADD: BINARY_OP(+); break;
	  case OP_SUBTRACT: BINARY_OP(-); break;
	  case OP_MULTIPLY: BINARY_OP(*); break;
	  case OP_DIVIDE: BINARY_OP(/); break;
	    // case OP_NEGATE: push(-pop()); break;
	  case OP_NEGATE:
	    *(vm.stackTop - 1) = -(*(vm.stackTop - 1));
	    break;
		
	  case OP_RETURN: {
	       printValue(pop());
	       printf("\n");
	       return INTERPRET_OK;
	  }
	  }
     }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  freeChunk(&chunk);
  return result;

}
