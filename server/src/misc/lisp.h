#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/nujel/nujel.h"

void lispInit     ();
int  parseCommand (uint c, const char *cmd);
void lispRecvSExpr(uint c, const packet *p);
void lispEvents   ();
lClosure *lispClientClosure(uint c);
