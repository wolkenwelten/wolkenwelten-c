#pragma once
#include "nujel.h"

lVal *lnfTime      (lClosure *c, lVal *v);
lVal *lnfStrftime  (lClosure *c, lVal *v);
lVal *lnfTimeMsecs (lClosure *c, lVal *v);

void lAddTimeFuncs(lClosure *c);
