#include "../src/chunk.h"
#include "../unity/src/unity.h"

void test_writeChunkGrow() {
  Chunk chunk;
  initChunk(&chunk);
  writeChunk(&chunk, 0xff, 0);

  TEST_ASSERT_EQUAL_INT(8, chunk.capacity);
  TEST_ASSERT_EQUAL_INT(1, chunk.count);
  TEST_ASSERT_EQUAL_UINT8(0xff, chunk.code[0]);
}

/*
void test_writeChunkGrow() {
  Chunk chunk;
  initChunk(&chunk);
  // It should grow here
  writeChunk(&chunk, 0xff, 0);
  // It should not grow here

  TEST_ASSERT_EQUAL_INT(
}
*/

void test_addConstant() { TEST_ASSERT(1); }

void test_getLine() { TEST_ASSERT(1); }

void test_writeConstant() { TEST_ASSERT(1); }

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_writeChunkGrow);
  RUN_TEST(test_addConstant);
  RUN_TEST(test_getLine);
  RUN_TEST(test_writeConstant);
  return UNITY_END();
}
