#pragma once
#include <stdbool.h>
#include <SDL.h>

void doTouchupdate(float *vx,float *vy,float *vz);
void touchEventHandler(const SDL_Event *e);
bool touchSneak();
bool touchMine();
bool touchActivate();
