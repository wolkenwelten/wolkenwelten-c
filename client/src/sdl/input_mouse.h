#pragma once
#include "../../../common/src/common.h"
#include <SDL.h>

bool mouseSneak        ();
bool mouseBoost        ();
bool mousePrimary      ();
bool mouseSecondary    ();
bool mouseTertiary     ();
bool mouseThrow        ();
void mouseEventHandler (const SDL_Event *e);
