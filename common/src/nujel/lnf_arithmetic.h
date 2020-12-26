#pragma once
#include "nujel.h"

lVal *lnfAdd(lClosure *c, lVal *v);
lVal *lnfSub(lClosure *c, lVal *v);
lVal *lnfMul(lClosure *c, lVal *v);
lVal *lnfDiv(lClosure *c, lVal *v);
lVal *lnfMod(lClosure *c, lVal *v);
