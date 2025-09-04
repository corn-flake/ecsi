#include "identifier.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../scanner.h"
#include "character_type_tests.h"
#include "hexadecimal.h"
#include "scanner_operations.h"

static void subsequents() {
    while (isSubsequent(peek())) {
        advance();
    }
}

static void dotSubsequent() {
    if (!isDotSubsequent(peek())) errorToken("Expected a dot subsequent.");
    advance();
}

static void signSubsequent() {
    if (!isSignSubsequent(peek())) errorToken("Expected a sign subsequent.");
    advance();
}

Token peculiarIdentifier(IdentifierVariant variant) {
    if (IDENTIFIER_PECULIAR_STARTS_WITH_DOT == variant) dotSubsequent();

    // The identifier is only one explicit sign, like '+'.
    if (!isSignSubsequent(peek()) && '.' != peek())
        return makeToken(TOKEN_IDENTIFIER);

    if (isSignSubsequent(peek())) {
        // Read the sign subsequent.
        signSubsequent();
        subsequents();
        return makeToken(TOKEN_IDENTIFIER);
    }

    if ('.' == peek()) {
        dotSubsequent();
        subsequents();
        return makeToken(TOKEN_IDENTIFIER);
    }

    // Unreached.
    fprintf(stderr, "Should not reach here in peculiarIdentifier.\n");
    assert(false);
}

static bool atMnemonicEscape() {
    if ('\\' != peek()) return false;

    switch (peekNext()) {
        case 'a':
        case 'b':
        case 't':
        case 'n':
        case 'r':
            return true;
        default:
            return false;
    }
}

static void mnemonicEscape() {
    advance();
    advance();
}

static bool atInlineHexEscape() {
    return !('\\' == peek() && 'x' == peekNext());
}

static void symbolElement() {
    if ('|' != peek() && '\\' != peek()) {
        advance();
        return;
    }

    if (atInlineHexEscape()) {
        inlineHexEscape();
        return;
    }

    if (atMnemonicEscape()) {
        mnemonicEscape();
        return;
    }
}

Token identifier(IdentifierVariant variant) {
    if (IDENTIFIER_STARTS_WITH_VERTICAL_LINE == variant) {
        while ('|' != peek()) {
            symbolElement();
        }
        advance();
        return makeToken(TOKEN_IDENTIFIER);
    }

    while (isSubsequent(peek())) {
        advance();
    }

    return makeToken(TOKEN_IDENTIFIER);
}
