#include <stdio.h>

#include "../src/chunk.h"
#include "../src/value.h"
#include "../src/vm.h"
#include "../unity/src/unity.h"
#include "debug.h"

Chunk chunk;

#define LINE 4

void setUp(void) {
    initVM();
    initChunk(&chunk);
}

void tearDown(void) {
    freeChunk(&chunk);
    freeVM();
}

void test_writeChunkGrow(void) {
    writeChunk(&chunk, 0xff, LINE);

    TEST_ASSERT_EQUAL_INT(8, chunk.capacity);
    TEST_ASSERT_EQUAL_INT(1, chunk.count);
    TEST_ASSERT_EQUAL_UINT8(0xFF, chunk.code[0]);
}

void test_addConstant(void) {
    addConstant(&chunk, NIL_VAL);
    TEST_ASSERT(IS_NIL(chunk.constants.values[0]));
}

void test_getLine(void) {
    int const PRIMARY_LINE = LINE;
    int const SECONDARY_LINE = 123;

    writeChunk(&chunk, 0xFF, SECONDARY_LINE);
    TEST_ASSERT_EQUAL_INT(SECONDARY_LINE,
                          chunk.lines.lineNumbers[0].lineNumber);
    TEST_ASSERT_EQUAL_INT(1, chunk.lines.lineNumbers[0].repeats);

    writeChunk(&chunk, 0xFF, SECONDARY_LINE);
    TEST_ASSERT_EQUAL_INT(SECONDARY_LINE,
                          chunk.lines.lineNumbers[0].lineNumber);
    TEST_ASSERT_EQUAL_INT(2, chunk.lines.lineNumbers[0].repeats);

    writeChunk(&chunk, 0xFF, PRIMARY_LINE);
    TEST_ASSERT_EQUAL_INT(PRIMARY_LINE, chunk.lines.lineNumbers[1].lineNumber);
    TEST_ASSERT_EQUAL_INT(1, chunk.lines.lineNumbers[1].repeats);

    writeChunk(&chunk, 0xFF, PRIMARY_LINE);
    TEST_ASSERT_EQUAL_INT(PRIMARY_LINE, chunk.lines.lineNumbers[1].lineNumber);
    TEST_ASSERT_EQUAL_INT(2, chunk.lines.lineNumbers[1].repeats);
}

void test_writeConstant(void) {
    int i = 0;
    for (; i < 256; i++) {
        writeConstant(&chunk, NUMBER_VAL(1), LINE);
        TEST_ASSERT_EQUAL_UINT8(OP_CONSTANT, chunk.code[2 * i]);
    }

    int opConstantLongStart = 2 * i;
    for (i = 0; i < 10; i++) {
        writeConstant(&chunk, NUMBER_VAL(1), LINE);
        TEST_ASSERT_EQUAL_UINT8(OP_CONSTANT_LONG,
                                chunk.code[opConstantLongStart + 4 * i]);
    }
    disassembleChunk(&chunk, "test_WriteConstant chunk");
}

#undef LINE

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_writeChunkGrow);
    RUN_TEST(test_addConstant);
    RUN_TEST(test_getLine);
    RUN_TEST(test_writeConstant);
    return UNITY_END();
}
