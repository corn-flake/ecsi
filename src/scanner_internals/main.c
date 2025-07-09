#include <stdio.h>
// This comment is here to prevent the formatter from putting #include
// <readline/xyz.h> before #include <stdio.h>, which causes an error because
// readline depends on stdio.
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>

#include "scanner.h"

#define PROMPT "(sicp_scanner)> "

static void printTokens() {
  Token token = scanToken();
  while (token.type != TOKEN_EOF) {
    printToken(&token);
    token = scanToken();
  }
}

static char *readFile(char *path) {
  FILE *file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "Could not open file \"%s\".\n", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  rewind(file);

  char *buffer = (char *)malloc(file_size + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
  if (bytes_read < file_size) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
  }
  buffer[bytes_read] = '\0';

  fclose(file);
  return buffer;
}

static int scanFile(char *path) {
  char *source = readFile(path);
  initScanner(source);
  printTokens();
  free(source);
  return 0;
}

static void interactive() {
  char *line = readline(PROMPT);
  while (line != NULL) {
    initScanner(line);
    printTokens();
    free(line);
    line = readline(PROMPT);
  }
}

int main(int argc, char *argv[]) {
  switch (argc) {
    case 1:
      interactive();
      break;
    case 2:
      return scanFile(argv[1]);
    default:
      printf("Usage: sicp_scanner [file]\n");
  }
  return 0;
}
