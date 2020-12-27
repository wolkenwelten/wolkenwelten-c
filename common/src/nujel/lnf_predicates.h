#pragma once
#include "nujel.h"

lVal *lnfLess(lClosure *c, lVal *v);
lVal *lnfLessEqual(lClosure *c, lVal *v);
lVal *lnfEqual(lClosure *c, lVal *v);
lVal *lnfGreater(lClosure *c, lVal *v);
lVal *lnfGreaterEqual(lClosure *c, lVal *v);
lVal *lnfZero(lClosure *c, lVal *v);
