#pragma once
#include "../../common/src/common.h"

extern bool quit;
extern char *ansiFG[16];
extern char *termColors[16];
extern char *termReset;

u64 getTicks();
void mainTick();
void mainInit();
