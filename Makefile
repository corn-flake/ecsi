CFLAGS := -Wall -Wextra -fanalyzer -g

clox: main.c memory.c debug.c chunk.c
	gcc main.c memory.c value.c debug.c chunk.c ${CFLAGS} -o clox
