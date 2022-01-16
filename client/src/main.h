#pragma once
#include <stdbool.h>

extern bool quit;
extern bool gameRunning;
extern bool singleplayer;
extern bool playerChunkActive;

void playerInit();
void playerFree();
