#pragma once
#include <stdbool.h>
#include <SDL.h>

bool keyboardSneak();
bool keyboardPrimary();
bool keyboardSecondary();
bool keyboardTertiary();

void doKeyboardupdate(float *vx,float *vy,float *vz);
void keyboardEventHandler(const SDL_Event *e);
