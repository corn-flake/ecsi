#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
  initVM();
  
  Chunk chunk;
  initChunk(&chunk);

  /*
  for (double i = 0; i < 500; i++) {
    writeConstant(&chunk, i, 123);
    writeChunk(&chunk, OP_NEGATE, 123);
  }
  */

  writeConstant(&chunk, 1.0, 1);
  writeConstant(&chunk, 2.0, 1);
  writeChunk(&chunk, OP_ADD, 1);

  writeConstant(&chunk, 3.0, 1);
  writeChunk(&chunk, OP_MULTIPLY, 1);

  writeConstant(&chunk, 2.0, 2);
  writeChunk(&chunk, OP_DIVIDE, 3);
  
  writeChunk(&chunk, OP_RETURN, 123);
  disassembleChunk(&chunk, "test chunk");
  interpret(&chunk);
  freeVM();
  freeChunk(&chunk);
  return 0;
}
