#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DO NOT SORT THEM!!
#include <readline/history.h>
#include <readline/readline.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

static void repl();
static void runFile(char const *path);
static char *readFile(char const *path);

int main(int argc, char const *argv[]) {
    initVM();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: clox [path]\n");
        exit(64);
    }

    freeVM();
    return 0;
}

static void repl() {
    char *line = readline("> ");
    while (line != NULL) {
        // Only add non-empty lines to the history
        if (*line) {
            add_history(line);
        }

        interpret(line);
        free(line);
        line = readline("> ");
    }
    puts("");
}

static void runFile(char const *path) {
    char *source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (INTERPRET_COMPILE_ERROR == result) exit(65);
    if (INTERPRET_RUNTIME_ERROR == result) exit(70);
}

static char *readFile(char const *path) {
    FILE *file = fopen(path, "rb");
    if (NULL == file) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(fileSize + 1);
    if (NULL == buffer) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}
