#pragma once
#include <stdbool.h>

extern char serverName[64];
extern float optionMusicVolume;
extern float optionSoundVolume;
extern int   optionWorldSeed;
extern bool  optionDebugInfo;

void parseOptions(int argc,char *argv[]);
void initOptions (int argc,char *argv[]);
void sanityCheckOptions();
