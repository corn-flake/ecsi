#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Commented to make interactive debugging easiser
// #define NAN_BOXING

// Disable spurious output for testing
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
#define DEBUG_STRESS_GC
#define DEBUG_LOG_GC

#define UINT8_COUNT (UINT8_MAX + 1)
#define ALLOW_UPPERCASE_HEX

/*
typedef struct {
  size_t count;
  size_t capacity;
} ArrayMetadata;
*/
