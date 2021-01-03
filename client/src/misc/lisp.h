#pragma once
#include "../../../common/src/common.h"

void lispInit        ();
void lispFree        ();
const char *lispEval (const char *str);
void lispRecvSExpr   (const packet *p);
