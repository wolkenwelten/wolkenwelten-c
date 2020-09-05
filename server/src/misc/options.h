#pragma once
#include <stdbool.h>

extern int  optionWorldSeed;
extern int  optionPort;
extern bool optionSingleplayer;
extern char optionSavegame[9];
extern bool verbose;

void parseOptions(int argc,const char *argv[]);
void initOptions (int argc,const char *argv[]);
void sanityCheckOptions();
