#pragma once
#include "../../../common/src/common.h"

void lispInit     ();
int  parseCommand (uint c, const char *cmd);
void lispRecvSExpr(uint c, const packet *p);
