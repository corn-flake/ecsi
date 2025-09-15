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
