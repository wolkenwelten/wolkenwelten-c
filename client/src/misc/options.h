#pragma once
#include "../../../common/src/common.h"

extern char  playerName[28];
extern char  serverName[64];
extern char  optionExecPath[256];
extern char  optionSavegame[32];
extern float optionSoundVolume;
extern int   optionWorldSeed;
extern bool  optionDebugInfo;
extern bool  optionMute;
extern float optionMouseSensitivy;
extern bool  optionThirdPerson;
extern bool  optionWireframe;

extern int   optionWindowWidth;
extern int   optionWindowHeight;
extern int   optionWindowX;
extern int   optionWindowY;
extern bool  optionFullscreen;

void parseOptions       (int argc,char *argv[]);
void initOptions        (int argc,char *argv[]);
void sanityCheckOptions ();
void saveOptions        ();
void loadOptions        ();
