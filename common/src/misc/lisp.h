#pragma once
#include "../common.h"
#include "../nujel/nujel.h"

extern lClosure *clRoot;

lClosure *lispCommonRoot  ();
void      lispDefineInt   (const char *symbol, int val);
void      lispDefineString(const char *symbol, char *str);
