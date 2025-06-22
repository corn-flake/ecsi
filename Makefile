DEBUG_ASAN_FLAGS := -Wall -Wextra -Wpedantic -fanalyzer -g -fsanitize=address 
DEBUG_NO_ASAN_FLAGS := -Wall -Wextra -Wpedantic -fanalyzer -g
RELEASE_FLAGS := -Wall -Wextra -Wpedantic -fanalyzer -O3 
EXEC_NAME := lisp_compiler
CORE_C_FILES := chunk.c compiler.c debug.c line_number.c main.c memory.c object.c table.c value.c vm.c
SCANNER_C_FILES := scanner/scanner.c scanner/character_type_tests.c scanner/hexadecimal.c scanner/identifier.c scanner/intertoken_space.c scanner/pound_something.c scanner/scan_booleans.c
PARSER_C_FILES := parser/parser.c parser/derived_expressions.c parser/token_to_type.c parser/literals.c
CC := gcc

lisp_compiler: ${CORE_C_FILES} ${SCANNER_C_FILES} ${PARSER_C_FILES}
	${CC} ${CORE_C_FILES} ${SCANNER_C_FILES} ${PARSER_C_FILES} ${DEBUG_ASAN_FLAGS} -o ${EXEC_NAME}

gdb: ${CORE_C_FILES} ${SCANNER_C_FILES} ${PARSER_C_FILES}
	${CC} ${CORE_C_FILES} ${SCANNER_C_FILES} ${PARSER_C_FILES} ${DEBUG_NO_ASAN_FLAGS} -o ${EXEC_NAME}_gdb

release: ${CORE_C_FILES} ${SCANNER_C_FILES} ${PARSER_C_FILES}
	${CC} ${CORE_C_FILES} ${SCANNER_C_FILES} ${PARSER_C_FILES} ${RELEASE_FLAGS} -o ${EXEC_NAME}_release

clean:
	rm ${EXEC_NAME} ${EXEC_NAME}_gdb
