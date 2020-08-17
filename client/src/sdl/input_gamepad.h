#pragma once
#include <stdbool.h>
#include <SDL.h>

bool gamepadSneak();
bool gamepadPrimary();
bool gamepadSecondary();
bool gamepadTertiary();

void gamepadInit();
void closeGamepad();
void checkForGamepad();
void checkForHaptic();
void controllerDeviceEvent(const SDL_Event *e);

void doGamepadMenuUpdate();
void doGamepadupdate(float *vx,float *vy,float *vz);
void gamepadEventHandler(const SDL_Event *e);

void vibrate(float force);
