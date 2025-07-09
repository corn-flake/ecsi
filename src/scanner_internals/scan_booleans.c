#include "scan_booleans.h"

#include "../scanner.h"
#include "scanner_operations.h"

Token scanTrue() {
  if (!match('r')) return makeToken(TOKEN_BOOLEAN);
  if (!matchString("ue")) return errorToken("Expect 'ue' after '#tr'");
  return makeToken(TOKEN_BOOLEAN);
}

Token scanFalse() {
  if (!match('a')) return makeToken(TOKEN_BOOLEAN);
  if (!matchString("lse")) return errorToken("Expect 'lse' after '#fa'");
  return makeToken(TOKEN_BOOLEAN);
}
