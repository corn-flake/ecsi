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

#include "token_to_type.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char characterNameToChar(Token const *token);

bool booleanTokenToBool(Token *token) {
    if (textOfTokenEqualToString(token, "#t") || textOfTokenEqualToString(token, "#true")) {
        return true;
    } else if (textOfTokenEqualToString(token, "#f") || textOfTokenEqualToString(token, "#false")) {
        return false;
    } else {
        // We crash the program if this happens because if it does, it's a
        // programmer error in the scanner.
        DIE("Boolean token must have '#t', '#f', '#true' or '#false' as its start field.");
    }
}

double numberTokenToDouble(Token *token) { return strtod(tokenGetStart(token), NULL); }

char characterTokenToChar(Token *token) {
    // All characters start with #\, so the start at 2 is the actual character.
    return tokenGetLength(token) > 3 ? characterNameToChar(token) : tokenGetStart(token)[2];
}

static char characterNameToChar(Token const *token) {
    if (textOfTokenEqualToString(token, "alarm")) return (char)0x07;
    if (textOfTokenEqualToString(token, "backspace")) return (char)0x08;
    if (textOfTokenEqualToString(token, "delete")) return (char)0x07F;
    if (textOfTokenEqualToString(token, "escape")) return (char)0x1B;
    if (textOfTokenEqualToString(token, "newline")) return '\n';
    if (textOfTokenEqualToString(token, "null")) return '\0';
    if (textOfTokenEqualToString(token, "return")) return (char)0x0D;
    if (textOfTokenEqualToString(token, "space")) return ' ';
    if (textOfTokenEqualToString(token, "tab")) return '\t';

    // We crash the program if this happens because if it does, it's a
    // programmer error in the scanner.
    DIE("Character name must be either 'alarm', 'backspace', 'delete', "
            "'escape', 'newline', 'null', 'return', 'space' or 'tab'.");
}
