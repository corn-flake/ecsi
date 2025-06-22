CFLAGS = -Wall -Wextra -Wpedantic -fanalyzer -fsanitize=address -g -lreadline
GDB_FLAGS = -Wall -Wextra -Wpedantic -fanalyzer -g -lreadline
SCANNER_C_FILES = ../scanner/character_type_tests.c ../scanner/hexadecimal.c ../scanner/identifier.c ../scanner/intertoken_space.c ../scanner/pound_something.c ../scanner/scan_booleans.c ../scanner/scanner.c
C_FILES = main.c parser.c ../value.c ../memory.c ../object.c ../table.c ../chunk.c ../vm.c
EXECUTABLE_NAME = sicp_parser
GDB_EXECUTABLE_NAME = sicp_parser_gdb

sicp_parser: ${C_FILES} ${SCANNER_C_FILES}
	gcc ${CFLAGS} ${C_FILES} ${SCANNER_C_FILES} -o ${EXECUTABLE_NAME}

gdb: ${C_FILES} ${SCANNER_C_FILES}
	gcc ${GDB_FLAGS} ${C_FILES} ${SCANNER_C_FILES} -o ${GDB_EXECUTABLE_NAME}
