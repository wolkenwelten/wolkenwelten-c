#pragma once
#include <stdbool.h>
#include <SDL.h>

void mouseEventHandler(const SDL_Event *e);
bool mouseSneak();
bool mouseMine();
bool mouseActivate();
