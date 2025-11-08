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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
  If defined, the Value data type will not be a tagged union.
  Instead, it will store non-number values by manipulating the
  unused bits in a NaN double.
 */
// #define NAN_BOXING

#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

// If defined, every allocation will trigger garbage collection.
#define DEBUG_STRESS_GC

/*
  If defined, prints log information about what the
  garbage collector is doing.
*/
// #define DEBUG_LOG_GC

// If defined, the VM will print the stack whenever the stack is changed.
#define DEBUG_STACK

#define UINT8_COUNT (UINT8_MAX + 1)

#define ERROR(...)                                                     \
    do {                                                               \
        fprintf(stderr, "(%s:%d) %s: ", __FILE__, __LINE__, __func__); \
        fprintf(stderr, __VA_ARGS__);                                  \
        putc('\n', stderr);                                            \
    } while (false)

#define DIE(...)            \
    do {                    \
        ERROR(__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (false)

#define UNREACHABLE() DIE("%s", "Reached unreachable code path.")
