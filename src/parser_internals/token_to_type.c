#include "token_to_type.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline size_t min(size_t a, size_t b);
static char characterNameToChar(Token const *token);

bool booleanTokenToBool(Token *token) {
    if (!memcmp(token->start, "#t", 2) || !memcmp(token->start, "#true", 5)) {
        return true;
    } else if (!memcmp(token->start, "#f", 2) ||
               !memcmp(token->start, "#false", 6)) {
        return false;
    }

    else {
        // We crash the program if this happens because if it does, it's a
        // programmer error in the scanner.
        fprintf(stderr,
                "Boolean token must have '#t', '#f', '#true' or "
                "'#false' as it's start field.");
        assert(false);
    }
}

static inline size_t min(size_t a, size_t b) { return a < b ? a : b; }

double numberTokenToDouble(Token *token) { return strtod(token->start, NULL); }

char characterTokenToChar(Token *token) {
    // All characters start with #\, so the start at 2 is the actual character.
    return token->length > 3 ? characterNameToChar(token) : token->start[2];
}

static char characterNameToChar(Token const *token) {
    if (!memcmp(token->start, "alarm", min(5, token->length)))
        return (char)0x07;
    if (!memcmp(token->start, "backspace", min(9, token->length))) {
        return (char)0x08;
    }
    if (!memcmp(token->start, "delete", 6)) return (char)0x7F;
    if (!memcmp(token->start, "escape", 6)) return (char)0x1B;
    if (!memcmp(token->start, "newline", 7)) return '\n';
    if (!memcmp(token->start, "null", 4)) return '\0';
    if (!memcmp(token->start, "return", 5)) return (char)0x0D;
    if (!memcmp(token->start, "space", 5)) return ' ';
    if (!memcmp(token->start, "tab", 3)) return '\t';

    // We crash the program if this happens because if it does, it's a
    // programmer error in the scanner.
    fprintf(stderr,
            "Character name must be either 'alarm', 'backspace', 'delete', "
            "'escape', 'newline', 'null', 'return', 'space' or 'tab'.");
    assert(false);
}
