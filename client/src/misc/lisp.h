#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/misc/lisp.h"

void lispKeyDown     (int code);
void lispInit        ();
void lispFree        ();
const char *lispEval (const char *str);
void lispRecvSExpr   (const packet *p);
void lispEvents      ();
