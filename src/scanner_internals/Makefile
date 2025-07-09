CFLAGS = -Wall -Wextra -Wpedantic -fanalyzer -fsanitize=address -g -lreadline -lhistory
GDB_FLAGS = -Wall -Wextra -Wpedantic -fanalyzer -g -lreadline -lhistory
EXECUTABLE_NAME = sicp_scanner
C_FILES = main.c scanner.c identifier.c intertoken_space.c character_type_tests.c scan_booleans.c pound_something.c hexadecimal.c ../memory.c ../vm.c ../compiler.c ../table.c ../object.c ../value.c ../chunk.c ../line_number.c ../debug.c ../parser/parser.c

sicp_scanner: ${C_FILES}
	gcc ${CFLAGS} ${C_FILES} -o ${EXECUTABLE_NAME}

gdb: ${C_FILES}
	gcc ${GDB_FLAGS} ${C_FILES} -o ${EXECUTABLE_NAME}
