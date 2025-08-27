#include "scanner_operations.h"

char peek() {
    if (isAtEnd()) return '\0';
    return *scanner.current;
}

char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

char advance() {
    scanner.current++;
    return scanner.current[-1];
}

bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;

    scanner.current++;
    return true;
}

bool matchString(char const *string) {
    size_t charsLeft = strlen(scanner.current);
    size_t stringLength = strlen(string);

    if (stringLength > charsLeft) return false;
    if (strncmp(string, scanner.current, stringLength) != 0) return false;

    for (size_t i = 0; i < stringLength; i++) advance();

    return true;
}

Token makeToken(TokenType type) {
    Token token = {
        .type = type,
        .start = scanner.start,
        .length = scanner.current - scanner.start,
        .line = scanner.line,
    };
    return token;
}

Token errorToken(char const *message) {
    Token token = {
        .type = TOKEN_ERROR,
        .start = message,
        .length = strlen(message),
        .line = scanner.line,
    };
    return token;
}

bool isAtEnd() { return *scanner.current == '\0'; }
