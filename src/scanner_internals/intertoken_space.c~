#include "intertoken_space.h"

#include <stdbool.h>

#include "character_type_tests.h"
#include "scanner.h"

static bool skipLineEndings() {
  bool skippedAnything = false;
  while (peek() == '\n' || peek() == '\r') {
    scanner.line++;
    advance();
    if (!skippedAnything) skippedAnything = true;
  }
  return skippedAnything;
}

static bool skipIntralineWhitespace() {
  bool skippedAnything = false;
  while (isIntralineWhitespace(peek())) {
    advance();
    if (!skippedAnything) skippedAnything = true;
  }
  return skippedAnything;
}

static bool skipWhitespace() {
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

static bool at_start_of_nested_comment() {
  return peek() == '#' && peekNext() == '|';
}

static bool at_end_of_nested_comment() {
  return peek() == '|' && peekNext() == '#';
}

static void skipNestedComment_text() {
  while (!at_end_of_nested_comment() && !at_start_of_nested_comment())
    advance();
}

/*
  We forward declare the function because it's mutally recursive with
  skipNestedComment.
*/
static void skipNestedComment_conts();

static bool skipNestedComment() {
  if (at_start_of_nested_comment()) {
    // Skip the start of the comment.
    advance();
    advance();

    // Skip the middle of the comment.
    skipNestedComment_text();
    skipNestedComment_conts();

    // Skip the end of the comment.
    advance();
    advance();

    return true;
  }

  return false;
}

static void skipNestedComment_conts() {
  skipNestedComment();
  skipNestedComment_text();
}

static bool skipLineComment() {
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

/*
  We forward declare this here because it's mutually recursive
  with skip_datum_comment.

  static void skipIntertokenSpace ();
*/

/*
static bool
skip_datum_comment ()
{
  if (peek () == '#' && peekNext () == ';')
    {
      skipIntertokenSpace ();
      // skip the datum.
      // TODO: Implement a datum scanner so that we can skip them.
      error_token ("Datum comments are currently not supported.");
      return true;
    }

  return false;
}
*/

static bool skipComment() {
  if (skipLineComment()) return true;

  if (skipNestedComment()) return true;

  /*
  if (skip_datum_comment ())
    return true;
  */

  return false;
}

/*
static bool
directive ()
{
  if (peek () == '#' && peek () == '!')
    {
      // TODO: Implement directives.
      error_token ("Directives are currently not supported.");
      return true;
    }

  return false;
}
*/

static bool skipAtmosphere() {
  if (skipWhitespace()) return true;

  if (skipComment()) return true;

  /*
  if (directive ())
    return true;
  */

  return false;
}

void skipIntertokenSpace() { while (skipAtmosphere()); }
