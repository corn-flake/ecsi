#include <stdio.h>

#include "../src/chunk.h"
#include "../src/value.h"
#include "../src/vm.h"
#include "../unity/src/unity.h"

void setUp() { initVM(); }

void tearDown() { freeVM(); }

void test_writeChunkGrow() {
    Chunk chunk;
    initChunk(&chunk);
    writeChunk(&chunk, 0xff, 0);

    TEST_ASSERT_EQUAL_INT(8, chunk.capacity);
    TEST_ASSERT_EQUAL_INT(1, chunk.count);
    TEST_ASSERT_EQUAL_UINT8(0xff, chunk.code[0]);
}

void test_addConstant() {
    Chunk chunk;
    initChunk(&chunk);

    addConstant(&chunk, NIL_VAL);
    TEST_ASSERT(IS_NIL(chunk.constants.values[0]));
}

void test_getLine() {
    Chunk chunk;
    initChunk(&chunk);

    writeChunk(&chunk, 0xFF, 123);
    TEST_ASSERT_EQUAL_INT(123, chunk.lines.lineNumbers[0].lineNumber);
    TEST_ASSERT_EQUAL_INT(1, chunk.lines.lineNumbers[0].repeats);

    writeChunk(&chunk, 0xFF, 123);
    TEST_ASSERT_EQUAL_INT(123, chunk.lines.lineNumbers[0].lineNumber);
    TEST_ASSERT_EQUAL_INT(2, chunk.lines.lineNumbers[0].repeats);

    writeChunk(&chunk, 0xFF, 4);
    TEST_ASSERT_EQUAL_INT(4, chunk.lines.lineNumbers[1].lineNumber);
    TEST_ASSERT_EQUAL_INT(1, chunk.lines.lineNumbers[1].repeats);

    writeChunk(&chunk, 0xFF, 4);
    TEST_ASSERT_EQUAL_INT(4, chunk.lines.lineNumbers[1].lineNumber);
    TEST_ASSERT_EQUAL_INT(2, chunk.lines.lineNumbers[1].repeats);
}

void test_writeConstant() {
    Chunk chunk;
    initChunk(&chunk);

    writeConstant(&chunk, NIL_VAL, 4);
    TEST_ASSERT_EQUAL_INT(OP_CONSTANT, chunk.code[0]);
    TEST_ASSERT(IS_NIL(chunk.constants.values[0]));

    for (int written = 1; written <= 256; written++) {
        writeConstant(&chunk, NIL_VAL, 4);
    }

    TEST_ASSERT_EQUAL_INT(OP_CONSTANT_LONG, chunk.code[chunk.count - 1]);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_writeChunkGrow);
    RUN_TEST(test_addConstant);
    RUN_TEST(test_getLine);
    RUN_TEST(test_writeConstant);
    return UNITY_END();
}
