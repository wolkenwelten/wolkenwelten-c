#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/nujel/nujel.h"
#include "../../../common/nujel/lib/api.h"

void      lispInit         ();
int       parseCommand     (uint pid, const char *cmd);
void      lispRecvSExpr    (uint pid, const packet *p);
lClosure *lispClientClosure(uint pid);
void      lispEvents       ();
