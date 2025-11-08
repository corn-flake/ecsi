/*
  Copyright 2025 Evan Cooney

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

#include "intertoken_space.h"

#include <stdbool.h>

#include "../scanner.h"
#include "character_type_tests.h"
#include "scanner_operations.h"

// Try to skip as many "line ending"s as possible and return true if anything
// was skipped.
static bool skipLineEndings(void);

/*
  Try to skip as much "intraline whitespace" as possible and return true if
  anything was skipped.
*/
static bool skipIntralineWhitespace(void);

/*
  Try to skip as much whitespace as possible and return true if anything was
  skipped
*/
static bool skipWhitespace(void);

// Return true if the scanner is at the start of a nested comment.
static bool atStartOfNestedComment(void);

// Return true if the scanner is at the end of a nested comment.
static bool atEndOfNestedComment(void);

// Try to skip the text of a nested comment.
static void skipNestedCommentText(void);

// Try to skip as many nested comment continuations as possible.
static void skipNestedCommentConts(void);

// Try to skip a nested comment, and return true if anything was skipped.
static bool skipNestedComment(void);

/*
  Try to skip a single line comment, and return true if anything was
  skipped.
 */
static bool skipLineComment(void);

// Try to skip a comment, and return true if anything was skipped.
static bool skipComment(void);

/*
 Try to skip as many atmosphere as possible, and return true if anything was
 skipped.
*/
static bool skipAtmosphere(void);

void skipIntertokenSpace(void) { while (skipAtmosphere()); }

static bool skipAtmosphere(void) {
    if (skipWhitespace()) return true;

    if (skipComment()) return true;

    return false;
}

static bool skipWhitespace(void) {
    bool skippedAnything = false;
    bool skipped;

    do {
        skipped = false;
        if (skipIntralineWhitespace()) {
            skippedAnything = true;
            skipped = true;
        }
        if (skipLineEndings()) {
            skippedAnything = true;
            skipped = true;
        }
    } while (skipped);

    return skippedAnything;
}

static bool skipIntralineWhitespace(void) {
    bool skippedAnything = false;
    while (isIntralineWhitespace(peek())) {
        advance();
        if (!skippedAnything) skippedAnything = true;
    }
    return skippedAnything;
}

static bool skipLineEndings(void) {
    bool skippedAnything = false;
    while (peek() == '\n' || peek() == '\r') {
        scanner.line++;
        advance();
        if (!skippedAnything) skippedAnything = true;
    }
    return skippedAnything;
}

static bool skipComment(void) {
    if (skipLineComment()) return true;

    if (skipNestedComment()) return true;

    return false;
}

static bool skipLineComment(void) {
    if (peek() == ';') {
        while (peek() != '\n' && peek() != '\r') advance();

        /*
          If the file has "\r\n" line endings, the while loop
          will stop advancing after the '\r', but the '\n' still has to
          be skipped, so we skip it here.
        */
        if (peek() == '\n') advance();

        return true;
    }

    return false;
}

static bool skipNestedComment(void) {
    if (atStartOfNestedComment()) {
        // Skip the start of the comment.
        advance();
        advance();

        // Skip the middle of the comment.
        skipNestedCommentText();
        skipNestedCommentConts();

        // Skip the end of the comment.
        advance();
        advance();

        return true;
    }

    return false;
}

static void skipNestedCommentConts(void) {
    skipNestedComment();
    skipNestedCommentText();
}

static void skipNestedCommentText(void) {
    while (!atEndOfNestedComment() && !atStartOfNestedComment()) advance();
}

static bool atStartOfNestedComment(void) {
    return peek() == '#' && peekNext() == '|';
}

static bool atEndOfNestedComment(void) {
    return peek() == '|' && peekNext() == '#';
}
