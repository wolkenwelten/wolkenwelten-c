#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/nujel/nujel.h"
#include "../../../common/nujel/lib/api.h"

void lispInputHandler(lSymbol *input, int key, int action);
void lispInputTick   ();
void lispInit        ();
void lispFree        ();
const char *lispEval (const char *str, bool humanReadable);
lVal *lispEvalL(lVal *expr);
void lispEvents      ();
