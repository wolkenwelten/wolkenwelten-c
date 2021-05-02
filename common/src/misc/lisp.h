#pragma once
#include "../common.h"
#include "../nujel/nujel.h"

extern lClosure *clRoot;

lClosure *lispCommonRoot  ();
void      lispDefineInt   (const char *symbol, int val);
void      lispDefineString(const char *symbol, char *str);
lVal     *lispCallFuncI   (const char *symbol, int ia);
lVal     *lispCallFuncIII (const char *symbol, int ia, int ib, int ic);
lVal     *lispCallFuncS   (const char *symbol, const char *str);
lVal     *lispCallFuncVII (const char *symbol, const vec va, int ib, int ic);
