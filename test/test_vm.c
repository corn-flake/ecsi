#include <stdio.h>

#include "../src/vm.h"
#include "../unity/src/unity.h"

void setUp(void) {
    puts("setting it up");
    initVM();
}

void tearDown(void) {
    puts("tearing it down");
    freeVM();
}

void testPop(void) { puts("testing pop"); }

void testPush(void) { puts("testing push"); }

void testInterpret(void) { puts("testing interpret"); }

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(testPop);
    RUN_TEST(testPush);
    RUN_TEST(testInterpret);
    return UNITY_END();
}
