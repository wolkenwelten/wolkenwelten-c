#pragma once
#include <stdbool.h>
#include <SDL.h>

void doKeyboardupdate(float *vx,float *vy,float *vz);
void keyboardEventHandler(const SDL_Event *e);
bool keyboardSneak();
bool keyboardMine();
bool keyboardActivate();
