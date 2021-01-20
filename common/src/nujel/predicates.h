#pragma once
#include "nujel.h"

lVal *lnfLess         (lClosure *c, lVal *v);
lVal *lnfLessEqual    (lClosure *c, lVal *v);
lVal *lnfEqual        (lClosure *c, lVal *v);
lVal *lnfGreater      (lClosure *c, lVal *v);
lVal *lnfGreaterEqual (lClosure *c, lVal *v);
lVal *lnfZero         (lClosure *c, lVal *v);

lVal *lnfIntPred      (lClosure *c, lVal *v);
lVal *lnfFloatPred    (lClosure *c, lVal *v);
lVal *lnfVecPred      (lClosure *c, lVal *v);
lVal *lnfNilPred      (lClosure *c, lVal *v);
lVal *lnfInfPred      (lClosure *c, lVal *v);
lVal *lnfNumberPred   (lClosure *c, lVal *v);
lVal *lnfStringPred   (lClosure *c, lVal *v);
lVal *lnfEmptyPred    (lClosure *c, lVal *v);
lVal *lnfPosPred      (lClosure *c, lVal *v);
lVal *lnfNegPred      (lClosure *c, lVal *v);
lVal *lnfPairPred     (lClosure *c, lVal *v);
