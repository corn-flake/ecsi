#pragma once

#include "chunk.h"
#include "object.h"

ObjFunction *compile(char const *source);
void markCompilerRoots();
