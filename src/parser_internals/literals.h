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

#include "../parser.h"
#include "parser_operations.h"

ExprLiteral *makeLiteral(bool isQuoted, Value value);

Value parseVectorUsing(ParseFn parse);
ExprLiteral *symbol(void);

ExprLiteral *parseBytevector(void);
ExprLiteral *parseNumber(void);
ExprLiteral *parseBoolean(void);
ExprLiteral *parseCharacter(void);
ExprLiteral *parseString(void);
ExprLiteral *parseNumberNoCheck(void);
ExprLiteral *parseBooleanNoCheck(void);
ExprLiteral *parseCharacterNoCheck(void);
ExprLiteral *parseStringNoCheck(void);
ExprLiteral *parseVector(void);
