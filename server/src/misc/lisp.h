#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/misc/lisp.h"
#include "../../../common/nujel/lib/nujel.h"

void      lispInit         ();
int       parseCommand     (uint c, const char *cmd);
void      lispRecvSExpr    (uint c, const packet *p);
lClosure *lispClientClosure(uint c);
void      lispEvents       ();
