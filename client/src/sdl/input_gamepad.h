#pragma once
#include "../../../common/src/common.h"
#include <SDL.h>

bool gamepadSneak          ();
bool gamepadPrimary        ();
bool gamepadSecondary      ();
bool gamepadTertiary       ();

void gamepadInit           ();
void closeGamepad          ();
void checkForGamepad       ();
void checkForHaptic        ();
void controllerDeviceEvent (const SDL_Event *e);

void doGamepadMenuUpdate   ();
vec  doGamepadupdate       (vec vel);
void gamepadEventHandler   (const SDL_Event *e);

void vibrate               (float force);
