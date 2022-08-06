#pragma once
#include <stdbool.h>

extern bool quit;
extern bool gameRunning;
extern bool singleplayer;
extern const char *gameModuleName;

void playerInit();
void playerFree();
void startGame(const char *moduleName);
void closeGame();
void exitCleanly();
