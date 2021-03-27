#pragma once
#include "nujel.h"

lVal *lnfNot (lClosure *c, lVal *v);
lVal *lnfAnd (lClosure *c, lVal *v);
lVal *lnfOr  (lClosure *c, lVal *v);

void lAddBooleanFuncs(lClosure *c);
