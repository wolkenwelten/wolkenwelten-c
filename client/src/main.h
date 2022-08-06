#pragma once
#include <stdbool.h>

extern bool quit;
extern bool gameRunning;
extern bool singleplayer;

void playerInit();
void playerFree();
void startGame(const char *moduleName);
void closeGame();
void exitCleanly();
