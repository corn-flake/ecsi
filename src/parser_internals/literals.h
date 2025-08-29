#pragma once

#include "../value.h"
#include "parser_operations.h"

Value parseVectorUsing(ParseFn parse);
Value symbol();
Value parseBytevector();
Value parseNumber();
Value parseBoolean();
Value parseCharacter();
Value parseString();
Value parseNumberNoCheck();
Value parseBooleanNoCheck();
Value parseCharacterNoCheck();
Value parseStringNoCheck();
