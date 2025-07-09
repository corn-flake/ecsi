#pragma once

#include "../scanner.h"

typedef enum {
  IDENTIFIER_PECULIAR_STARTS_WITH_DOT,
  IDENTIFIER_PECULIAR_NO_DOT,
  IDENTIFIER_STARTS_WITH_VERTICAL_LINE,
  IDENTIFIER_NO_VERTICAL_LINE,
} IdentifierVariant;

Token identifier(IdentifierVariant variant);
Token peculiarIdentifier(IdentifierVariant variant);
