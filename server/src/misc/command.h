#pragma once
#include "../../../common/src/common.h"

void initCommands  ();
void freeCommands  ();
int parseCommand   (int c, const char *cmd);
void lispRecvSExpr (int c,const packet *p);
