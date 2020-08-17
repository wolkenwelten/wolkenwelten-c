#pragma once
#include <stdbool.h>
#include <SDL.h>

bool touchSneak();
bool touchPrimary();
bool touchSecondary();
bool touchTertiary();

void doTouchupdate(float *vx,float *vy,float *vz);
void touchEventHandler(const SDL_Event *e);
