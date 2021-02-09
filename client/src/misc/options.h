#pragma once
#include "../../../common/src/common.h"

extern char  playerName[28];
extern char  serverName[64];
extern char  optionSavegame[32];
extern float optionMusicVolume;
extern float optionSoundVolume;
extern int   optionWorldSeed;
extern bool  optionDebugInfo;
extern bool  optionRuntimeReloading;
extern bool  optionMute;
extern float optionMouseSensitivy;
extern bool  optionThirdPerson;
extern int   optionAutomatedTest;
extern bool  optionWireframe;

extern int   optionWindowOrientation;
extern int   optionWindowWidth;
extern int   optionWindowHeight;
extern bool  optionFullscreen;

void parseOptions       (int argc,char *argv[]);
void initOptions        (int argc,char *argv[]);
void sanityCheckOptions ();
void saveOptions        ();
void loadOptions        ();
