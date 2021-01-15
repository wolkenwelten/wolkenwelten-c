#pragma once
#include "nujel.h"

lVal *lnfInt    (lClosure *c, lVal *v);
lVal *lnfFloat  (lClosure *c, lVal *v);
lVal *lnfVec    (lClosure *c, lVal *v);
lVal *lnfInf    (lClosure *c, lVal *v);
lVal *lnfBool   (lClosure *c, lVal *v);
lVal *lnfString (lClosure *c, lVal *v);
