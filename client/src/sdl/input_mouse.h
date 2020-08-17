#pragma once
#include <stdbool.h>
#include <SDL.h>

bool mouseSneak();
bool mousePrimary();
bool mouseSecondary();
bool mouseTertiary();

void mouseEventHandler(const SDL_Event *e);
