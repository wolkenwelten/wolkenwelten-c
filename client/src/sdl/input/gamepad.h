#pragma once
#include "../../../../common/src/common.h"
#include <SDL.h>

bool gamepadSneak          ();
bool gamepadBoost          ();
bool gamepadPrimary        ();
bool gamepadSecondary      ();
bool gamepadTertiary       ();
bool gamepadThrow          ();

void gamepadInit           ();
void closeGamepad          ();
void checkForGamepad       ();
void checkForHaptic        ();
void controllerDeviceEvent (const SDL_Event *e);

void doGamepadMenuUpdate   ();
vec  doGamepadupdate       (vec vel);
void gamepadEventHandler   (const SDL_Event *e);

void vibrate               (float force);
