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

static void showStartupCopyingNotice();
static void showW();
static void showC();

int main(int argc, char const *argv[]) {
    showStartupCopyingNotice();
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

        if (!strcmp(line, "show c")) {
            showC();
        } else if (!strcmp(line, "show w")) {
            showW();
        } else {
            interpret(line);
        }

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

static void showStartupCopyingNotice() {
    puts(
        "Ecsi Copyright (C) 2025 Evan Cooney\n"
        "This program comes with ABSOLUTELY NO WARRANTY; for details type "
        "'show w'.\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions; type 'show c' for details.");
}

static void showW() {
    // This message was copied from the output of the ,show w
    // command on GNU Guile 3.0, but the word 'Guile' was replaced with 'Ecsi'.
    puts(
        "Ecsi is distributed WITHOUT ANY WARRANTY. The following\n"
        "sections from the GNU General Public License, version 3, should\n"
        "make that clear.\n\n"
        "\t15. Disclaimer of Warranty.\n\n"
        "\tTHERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY\n"
        "APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE "
        "COPYRIGHT\n"
        "HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM AS IS"
        " WITHOUT WARRANTY\n"
        "OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED "
        "TO,\n"
        "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A "
        "PARTICULAR\n"
        "PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE "
        "PROGRAM\n"
        "IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST "
        "OF\n"
        "ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n"
        "\t16. Limitation of Liability.\n\n"
        "\tIN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN "
        "WRITING\n"
        "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND/OR "
        "CONVEYS\n"
        "THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, "
        "INCLUDING ANY\n"
        "GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF "
        "THE\n"
        "USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO "
        "LOSS OF\n"
        "DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR "
        "THIRD\n"
        "PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER "
        "PROGRAMS),\n"
        "EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE "
        "POSSIBILITY OF\n"
        "SUCH DAMAGES.\n\n"
        "\t17. Interpretation of Sections 15 and 16.\n\n"

        "\tIf the disclaimer of warranty and limitation of liability provided\n"
        "above cannot be given local legal effect according to their terms,\n"
        "reviewing courts shall apply local law that most closely "
        "approximates\n"
        "an absolute waiver of all civil liability in connection with the\n"
        "Program, unless a warranty or assumption of liability accompanies a\n"
        "copy of the Program in return for a fee.\n"
        "\n"
        "See <http://www.gnu.org/licenses/lgpl.html>, for more details.\n");
}

static void showC() {
    // This was copied from the output of the ,show c command from GNU
    // Guile 3.0, but the word 'Guile' was replaced with the word 'Ecsi'.
    puts(
        "Ecsi is free software: you can redistribute it and/or modify\n"
        "it under the terms of the GNU Lesser General Public License as\n"
        "published by the Free Software Foundation, either version 3 of\n"
        "the License, or (at your option) any later version.\n"
        "\n"
        "Guile is distributed in the hope that it will be useful, but\n"
        "WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU\n"
        "Lesser General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU Lesser General Public\n"
        "License along with this program. If not, see\n"
        "<http://www.gnu.org/licenses/lgpl.html>.\n");
}
