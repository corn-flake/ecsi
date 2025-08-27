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

# Determine the C files that $(1) depends on.
# define get_dependencies_of_c_file
# grep -oE '#include ".*"' $(1) | cut -d'"' -f2 | sed 's/.h/.c/'
# endef

# Path to Unity source code
PATHU = unity/src/

# Path to my source code
PATHS = src/

# Path to tests
PATHT = test/

# Path to build directory
PATHB = build/

# Path to build results directory
PATHD = build/depends/

# Path to build temporary object files
PATHO = build/objs/

# Path to build results
PATHR = build/results/

chunk.o: chunk.c chunk.h line_number.c line_number.h memory.c memory.h value.c value.h vm.c vm.h

compiler.o: compiler.c compiler.h chunk.c chunk.h common.c common.h memory.c memory.h object.c object.h parser.c parser.h

debug.o: debug.c debug.h chunk.c chunk.h object.c object.h value.c value.h

line_number.o: line_number.c line_number.h memory.c memory.h

main.o: main.c chunk.c chunk.h common.h debug.c debug.h vm.c vm.h

memory.o: memory.c memory.h compiler.c compiler.h object.c object.h parser.c parser.h table.c table.h value.c value.h vm.c vm.h

object.o: object.c object.h memory.c memory.h table.c table.h value.c value.h vm.c vm.h

parser.o: parser.c parser.h memory.c memory.h object.c object.h parser_internals/derived_expressions.c parser_internals/derived_expressions.h parser_internals/literals.c parser_internals/literals.h parser_internals/parser_operations.c parser_internals/parser_operations.h parser_internals/token_to_type.h scanner.c scanner.h value.c value.h vm.c vm.h

scanner.o: scanner.c scanner.h memory.c memory.h object.c object.h scanner_internals/character_type_tests.h scanner_internals/character_type_tests.c scanner_internals/identifier.c scanner_internals/identifier.h scanner_internals/intertoken_space.c scanner_internals/intertoken_space.h scanner_internals/pound_something.c scanner_internals/pound_something.h scanner_internals/scanner_operations.c scanner_internals/scanner_operations.h

table.o: table.c table.h memory.c memory.h object.c object.h value.c value.h

value.o: value.c value.h memory.c memory.h object.c object.h

vm.o: vm.c vm.h chunk.c chunk.h common.h compiler.c compiler.h debug.c debug.h memory.c memory.h object.c object.h table.c table.h value.c value.h

parser_internals/derived_expressions.o: parser_internals/derived_expressions.c parser_internals/derived_expressions.h object.c object.h parser.c parser.h vm.c vm.h parser_internals/literals.h parser_internals/parser_operations.h

parser_internals/literals.o: parser_internals/literals.c parser_internals/literals.h object.c object.h parser.c parser.h parser_internals/parser_operations.c parser_internals/parser_operations.h parser_internals/token_to_type.c parser_internals/token_to_type.h

parser_internals/parser_operations.o: parser_internals/parser_operations.c parser_internals/parser_operations.h object.c object.h parser.c parser.h value.c value.h vm.c vm.h

parser_internals/token_to_type.o: parser_internals/token_to_type.c parser_internals/token_to_type.h 

scanner_internals/character_type_tests.o: scanner_internals/character_type_tests.c scanner_internals/character_type_tests.h

scanner_internals/hexadecimal.o: scanner_internals/hexadecimal.c scanner_internals/hexadecimal.o scanner.c scanner.h scanner_internals/character_type_tests.c scanner_internals/character_type_tests.o scanner_internals/scanner_operations scanner_internals/scanner_operations.h

scanner_internals/identifier.o: scanner_internals/identifier.c scanner_internals/identifier.h scanner.c scanner.h scanner_internals/character_type_tests.c scanner_internals/character_type_tests.h scanner_internals/hexadecimal.c scanner_internals/hexadecimal.h scanner_internals/scanner_operations.c scanner_internals/scanner_operations.h

scanner_internals/intertoken_space.o: scanner_internals/intertoken_space.c scanner_internals/intertoken_space.h scanner.c scanner.h scanner_internals/character_type_tests.c scanner_internals/character_type_tests.h scanner_internals/scanner_operations.c scanner_internals/scanner_operations.h

scanner_internals/pound_something.o: scanner_internals/pound_something.c scanner_internals/pound_something.h scanner.c scanner.h scanner_internals/character_type_tests.c scanner_internals/character_type_tests.h scanner_internals/hexadecimal.c scanner_internals/hexadecimal.h scanner_internals/scan_booleans.c scanner_internals/scan_booleans.h scanner_internals/scanner_operations.c scanner_internals/scanner_operations.h

scanner_internals/scan_booleans.o: scanner_internals/scan_booleans.c scanner_internals/scan_booleans.h scanner.c scanner.h scanner_internals/scanner_operations.c scanner_internals/scanner_operations.h

scanner_internals/scanner_operations.o: scanner_internals/scanner_operations.c scanner_internals/scanner_operations.h

_OBJS_NO_MAIN = chunk.o compiler.o debug.o line_number.o memory.o object.o parser.o scanner.o table.o value.o vm.o parser_internals/derived_expressions.o parser_internals/literals.o parser_internals/parser_operations.o parser_internals/token_to_type.o scanner_internals/character_type_tests.o scanner_internals/hexadecimal.o scanner_internals/identifier.o scanner_internals/intertoken_space.o scanner_internals/pound_something.o scanner_internals/scan_booleans.o scanner_internals/scanner_operations.o

_OBJS =  $(_OBJS_NO_MAIN) main.o

# Paths of object files
OBJS = $(patsubst %,$(PATHO)%,$(_OBJS))
OBJS_NO_MAIN = $(patsubst %,$(PATHO)%,$(_OBJS_NO_MAIN))

# All build paths
BUILD_PATHS = $(PATHB) $(PATHD) $(PATHO) $(PATHR)

# All test source files
SRCT = $(wildcard $(PATHT)*.c)

# The prefix that test files should start with
# Ex. chunk.c -> test_chunk.c
TEST_PREFIX = test_

# Executable name
EXECUTABLE_NAME = lisp_compiler

COMPILE = gcc -c
# COMPILE_DEBUG = $(COMPILE) -g3
LINK=gcc -lm -lreadline -fsanitize=address
DEPEND=gcc -MM -MG -MF
CFLAGS=-I. -I$(PATHU) -I$(PATHS) -DTEST -Wall -Wextra -Wpedantic -fanalyzer -g3 -fsanitize=address
# DEBUG_FLAGS=$(CFLAGS) -g3

# Find all .c files and replace the .c with a .o to generate the object files.
# _OBJS = $(shell cd ./src; find ./ -regex '^.*\.c$$' | sed 's/\.c/.o/'; cd ..)

# $(info _OBJS is $(_OBJS))

RESULTS = $(patsubst $(PATHT)$(TEST_PREFIX)%.c,$(PATHR)$(TEST_PREFIX)%.txt,$(SRCT) )
$(info RESULTS is $(RESULTS))

PASSED = `grep -s PASS $(PATHR)*.txt`
FAIL = `grep -s FAIL $(PATHR)*.txt`
IGNORE = `grep -s IGNORE $(PATHR)*.txt`

all: test compile install

test: $(BUILD_PATHS) $(RESULTS)
	@echo -e "-----------------------\nIGNORES:\n-----------------------"
	@echo -e "$(IGNORE)"
	@echo -e "-----------------------\nFAILURES:\n-----------------------"
	@echo -e "$(FAIL)"
	@echo -e "-----------------------\nPASSED:\n-----------------------"
	@echo -e "$(PASSED)"
	@echo -e "\nDONE"

$(PATHR)%.txt: $(PATHB)%.$(TARGET_EXTENSION)
	-./$< > $@ 2>&1

$(PATHB)$(TEST_PREFIX)%.$(TARGET_EXTENSION): $(PATHO)$(TEST_PREFIX)%.o $(PATHO)%.o $(PATHO)unity.o $(OBJS_NO_MAIN) #$(PATHD)$(TEST_PREFIX)%.d
	$(LINK) -o $@ $^

$(PATHO)%.o:: $(PATHT)%.c $(PATHO)/parser_internals $(PATHO)/scanner_internals
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHO)%.o:: $(PATHS)%.c $(PATHO)/parser_internals $(PATHO)/scanner_internals
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHO)%.o:: $(PATHU)%.c $(PATHU)%.h $(PATHO)/parser_internals $(PATHO)/scanner_internals
	$(COMPILE) $(CFLAGS) $< -o $@

$(PATHO)/parser_internals:
	$(MKDIR) $(PATHO)/parser_internals

$(PATHO)/scanner_internals:
	$(MKDIR) $(PATHO)/scanner_internals

$(PATHD)%.d:: $(PATHT)%.c
	$(DEPEND) $@ $<

$(PATHB):
	$(MKDIR) $(PATHB)

$(PATHD):
	$(MKDIR) $(PATHD)

$(PATHO):
	$(MKDIR) $(PATHO)

$(PATHR):
	$(MKDIR) $(PATHR)

debug: $(OBJS)

compile: $(OBJS)

install: $(OBJS)
	$(LINK) -o $(EXECUTABLE_NAME).$(TARGET_EXTENSION) $(OBJS)

format:
	clang-format -i $(PATHS)*.c $(PATHS)*.h $(PATHT)*.c

clean:
	$(CLEANUP) $(PATHO)*.o
	$(CLEANUP) $(PATHB)*.$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHR)*.txt
	$(CLEANUP) $(EXECUTABLE_NAME).$(TARGET_EXTENSION)
	$(CLEANUP) $(PATHS)*~ $(PATHS)/parser_internals/*~ $(PATHS)/scanner_internals/*~ $(PATHT)*~

.PRECIOUS: $(PATHB)$(TEST_PREFIX)%.$(TARGET_EXTENSION)
.PRECIOUS: $(PATHD)%.d
.PRECIOUS: $(PATHO)%.o
.PRECIOUS: $(PATHR)%.txt
