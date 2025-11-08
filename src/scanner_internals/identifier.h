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

#pragma once

#include "../scanner.h"

/*
  Different possible types of identifiers to specify which type to try
  to scan.
*/
typedef enum {
    // A peculiar identifier that starts with a dot.
    IDENTIFIER_PECULIAR_STARTS_WITH_DOT,

    IDENTIFIER_PECULIAR_NO_DOT,

    // An identifier that starts with a vertical line
    IDENTIFIER_STARTS_WITH_VERTICAL_LINE,

    // An identifier that doesn't start with a vertical line.
    IDENTIFIER_NO_VERTICAL_LINE,
} IdentifierVariant;

// Scan and return an identifier token.
Token identifier(IdentifierVariant variant);

// Scan and return an identifier token which is a peculiar identifier.
Token peculiarIdentifier(IdentifierVariant variant);
