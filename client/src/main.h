#pragma once
#include <stdbool.h>

extern bool quit;
extern bool gameRunning;
extern bool singleplayer;
extern bool playerChunkActive;
extern bool chnkChngOverflow;

void playerInit();
void playerFree();
