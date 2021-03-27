#pragma once
#include "nujel.h"

lVal *lnfLogAnd (lClosure *c, lVal *v);
lVal *lnfLogIor (lClosure *c, lVal *v);
lVal *lnfLogXor (lClosure *c, lVal *v);
lVal *lnfLogNot (lClosure *c, lVal *v);
lVal *lnfAsh    (lClosure *c, lVal *v);

void lAddBinaryFuncs(lClosure *c);
