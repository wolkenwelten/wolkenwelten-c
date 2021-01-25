#pragma once
#include "nujel.h"

lVal *lnfArrLength (lClosure *c, lVal *v);
lVal *lnfArrRef    (lClosure *c, lVal *v);
lVal *lnfArrSet    (lClosure *c, lVal *v);
lVal *lnfArrNew    (lClosure *c, lVal *v);
lVal *lnfArr       (lClosure *c, lVal *v);
