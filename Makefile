# Copyright 2025 Evan Cooney

# This file is part of Ecsi.

# Ecsi is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.

# Ecsi is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along with
# Ecsi. If not, see <https://www.gnu.org/licenses/>.

ifeq ($(OS),Windows_NT)
  ifeq ($(shell uname -s),) # not in a bash-like shell
	CLEANUP = del /F /Q
	MKDIR = mkdir
  else # in a bash-like shell, like msys
	CLEANUP = rm -f
	MKDIR = mkdir -p
  endif
	TARGET_EXTENSION=exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=out
endif

.PHONY: clean
.PHONY: test
.PHONY: compile
.PHONY: install

# Path to Unity source code
UNITY_PATH = unity/src/

# Path to my source code
SOURCE_PATH = src/

# Path to tests
TEST_PATH = test/

# Path to build directory
BUILD_PATH = build/

# Path to build results directory
DEPENDS_PATH = build/depends/

# Path to build temporary object files
OBJS_PATH = build/objs/

# Path to build results
BUILD_RESULTS_PATH = build/results/

# Subdirectories in build/objs/
OBJ_SUBDIRS = $(addprefix $(OBJS_PATH)/, parser_internals/ scanner_internals/)

_OBJS_NO_MAIN = smart_array.o chunk.o compiler.o debug.o line_number.o memory.o object.o parser.o scanner.o table.o value.o vm.o parser_internals/derived_expressions.o parser_internals/literals.o parser_internals/parser_operations.o parser_internals/token_to_type.o scanner_internals/character_type_tests.o scanner_internals/hexadecimal.o scanner_internals/identifier.o scanner_internals/intertoken_space.o scanner_internals/pound_something.o scanner_internals/scan_booleans.o scanner_internals/scanner_operations.o

_OBJS =  $(_OBJS_NO_MAIN) main.o

# Paths of object files
OBJS = $(patsubst %,$(OBJS_PATH)%,$(_OBJS))
OBJS_NO_MAIN = $(patsubst %,$(OBJS_PATH)%,$(_OBJS_NO_MAIN))

# All build paths
BUILD_SOURCE_PATH = $(BUILD_PATH) $(DEPENDS_PATH) $(OBJS_PATH) $(BUILD_RESULTS_PATH)

# All test source files
TEST_SOURCES = $(wildcard $(TEST_PATH)*.c)

# The prefix that test files should start with
# Ex. chunk.c -> test_chunk.c
TEST_PREFIX = test_

# Executable name
EXECUTABLE_NAME = ecsi

CC = gcc
COMPILE = $(CC) -c
LINK=$(CC) -lm -lreadline
DEPEND=gcc -MM -MG -MF
CFLAGS=-I. -I$(UNITY_PATH) -I$(SOURCE_PATH) -DTEST -Wall -Wextra -Wpedantic -fanalyzer -g3 -std=gnu11

RESULTS = $(patsubst $(TEST_PATH)$(TEST_PREFIX)%.c,$(BUILD_RESULTS_PATH)$(TEST_PREFIX)%.txt,$(TEST_SOURCES) )

PASSED = `grep -s PASS $(BUILD_RESULTS_PATH)*.txt`
FAIL = `grep -s FAIL $(BUILD_RESULTS_PATH)*.txt`
IGNORE = `grep -s IGNORE $(BUILD_RESULTS_PATH)*.txt`

all: test compile install

test: $(BUILD_SOURCE_PATH) $(RESULTS)
	@echo -e "-----------------------\nIGNORES:\n-----------------------"
	@echo -e "$(IGNORE)"
	@echo -e "-----------------------\nFAILURES:\n-----------------------"
	@echo -e "$(FAIL)"
	@echo -e "-----------------------\nPASSED:\n-----------------------"
	@echo -e "$(PASSED)"
	@echo -e "\nDONE"

$(BUILD_RESULTS_PATH)%.txt: $(BUILD_PATH)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

$(BUILD_PATH)$(TEST_PREFIX)%.$(TARGET_EXTENSION): $(OBJS_PATH)$(TEST_PREFIX)%.o $(OBJS_PATH)%.o $(OBJS_PATH)unity.o $(OBJS_NO_MAIN) #$(DEPENDS_PATH)$(TEST_PREFIX)%.d
	$(LINK) -o $@ $^

$(OBJS_PATH)%.o:: $(TEST_PATH)%.c | $(OBJ_SUBDIRS)
	$(COMPILE) $(CFLAGS) $< -o $@

$(OBJS_PATH)%.o:: $(SOURCE_PATH)%.c | $(OBJ_SUBDIRS)
	$(COMPILE) $(CFLAGS) $< -o $@

$(OBJS_PATH)%.o:: $(UNITY_PATH)%.c $(UNITY_PATH)%.h | $(OBJ_SUBDIRS)
	$(COMPILE) $(CFLAGS) $< -o $@

$(OBJ_SUBDIRS): | $(OBJS_PATH)
	$(MKDIR) $(OBJ_SUBDIRS)

$(DEPENDS_PATH)%.d:: $(TEST_PATH)%.c
	$(DEPEND) $@ $<

$(BUILD_PATH):
	$(MKDIR) $(BUILD_PATH)

$(DEPENDS_PATH):
	$(MKDIR) $(DEPENDS_PATH)

$(OBJS_PATH):
	$(MKDIR) $(OBJS_PATH)

$(BUILD_RESULTS_PATH):
	$(MKDIR) $(BUILD_RESULTS_PATH)

debug: $(OBJS)

compile: $(OBJS)

install: $(OBJS)
	$(LINK) -o $(EXECUTABLE_NAME).$(TARGET_EXTENSION) $(OBJS)

chunk.o: chunk.c line_number.c memory.c value.c vm.c smart_array.c

compiler.o: compiler.c chunk.c common.c memory.c object.c parser.c 

debug.o: debug.c chunk.c object.c value.c smart_array.c

line_number.o: line_number.c memory.c smart_array.c

main.o: main.c chunk.c debug.c vm.c 

memory.o: memory.c compiler.c object.c parser.c table.c value.c vm.c common.h

object.o: object.c memory.c table.c value.c vm.c 

parser.o: parser.c memory.c object.c parser_internals/derived_expressions.c parser_internals/literals.c parser_internals/parser_operations.c scanner.c value.c vm.c smart_array.c

scanner.o: scanner.c memory.c object.c scanner_internals/character_type_tests.c scanner_internals/identifier.c scanner_internals/intertoken_space.c scanner_internals/pound_something.c scanner_internals/scanner_operations.c 

table.o: table.c memory.c object.c value.c

value.o: value.c memory.c object.c smart_array.c

vm.o: vm.c chunk.c compiler.c debug.c memory.c object.c table.c value.c smart_array.c

parser_internals/derived_expressions.o: parser_internals/derived_expressions.c object.c parser.c vm.c

parser_internals/literals.o: parser_internals/literals.c object.c parser.c parser_internals/parser_operations.c parser_internals/token_to_type.c

parser_internals/parser_operations.o: parser_internals/parser_operations.c object.c parser.c value.c vm.c

parser_internals/token_to_type.o: parser_internals/token_to_type.c 

scanner_internals/character_type_tests.o: scanner_internals/character_type_tests.c

scanner_internals/hexadecimal.o: scanner_internals/hexadecimal.c scanner.c scanner_internals/character_type_tests.c scanner_internals/scanner_operations.c 

scanner_internals/identifier.o: scanner_internals/identifier.c scanner.c scanner_internals/character_type_tests.c scanner_internals/hexadecimal.c scanner_internals/scanner_operations.c

scanner_internals/intertoken_space.o: scanner_internals/intertoken_space.c scanner.c scanner_internals/character_type_tests.c scanner_internals/scanner_operations.c

scanner_internals/pound_something.o: scanner_internals/pound_something.c scanner.c scanner_internals/character_type_tests.c scanner_internals/hexadecimal.c scanner_internals/scan_booleans.c scanner_internals/scanner_operations.c

scanner_internals/scan_booleans.o: scanner_internals/scan_booleans.c scanner.c scanner_internals/scanner_operations.c 

scanner_internals/scanner_operations.o: scanner_internals/scanner_operations.c

smart_array.o: memory.c smart_array.c

format:
	clang-format -i $(SOURCE_PATH)*.c $(SOURCE_PATH)*.h $(TEST_PATH)*.c

clean:
	$(CLEANUP) $(OBJS_PATH)*.o
	$(CLEANUP) $(OBJS_PATH)/scanner_internals/*.o
	$(CLEANUP) $(OBJS_PATH)/parser_internals/*.o
	$(CLEANUP) $(BUILD_PATH)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(BUILD_RESULTS_PATH)*.txt
	$(CLEANUP) $(EXECUTABLE_NAME).$(TARGET_EXTENSION)
	$(CLEANUP) $(SOURCE_PATH)*~ $(SOURCE_PATH)/parser_internals/*~ $(SOURCE_PATH)/scanner_internals/*~ $(TEST_PATH)*~

.PRECIOUS: $(BUILD_PATH)$(TEST_PREFIX)%.$(TARGET_EXTENSION)
.PRECIOUS: $(DEPENDS_PATH)%.d
.PRECIOUS: $(OBJS_PATH)%.o
.PRECIOUS: $(BUILD_RESULTS_PATH)%.txt
