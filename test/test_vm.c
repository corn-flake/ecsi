#include <stdbool.h>

#include "../src/vm.h"
#include "../unity/src/unity.h"

void setUp() { initVM(); }
void tearDown() { freeVM(); }

void test_pop() {
    Value cases[5] = {NIL_VAL, NUMBER_VAL(5), BOOL_VAL(true), NUMBER_VAL(1.23),
                      BOOL_VAL(false)};

    for (int i = 0; i < 5; i++) {
        push(cases[i]);
        TEST_ASSERT(valuesEqual(cases[i], pop()));
    }
}

void test_push() {
    int i = 0;
    for (; i < 100000; i++) {
        push(NIL_VAL);
    }

    for (; i >= 0; i--) {
        pop();
    }
}

void test_interpret() {}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_pop);
    RUN_TEST(test_push);
    return UNITY_END();
}
