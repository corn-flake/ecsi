CFLAGS := -Wall -Wextra -fanalyzer -g 

clox: chunk.c compiler.c debug.c line_number.c main.c memory.c object.c scanner.c table.c value.c vm.c
	gcc chunk.c compiler.c debug.c line_number.c main.c memory.c object.c scanner.c table.c value.c vm.c ${CFLAGS} -o clox

clean:
	rm clox
