#pragma once
#include "../../../common/src/common.h"

extern uint  frameTimeLast;
extern uint  frameCountSinceLast;
extern float curFPS;
extern uint  worstFrame;
extern bool  sdlGamepadInit;
extern bool  sdlHapticInit;

void fpsTick();
void initSDL();
void handleEvents();
void sdlResize(int newW,int newH);
void closeSDL();
void setFullscreen(bool fs);

void swapWindow();
void setRelativeMouseMode(bool ra);
void warpMouse(int nMouseX,int nMouseY);
unsigned int getMouseState(int *mx, int *my);
unsigned int getTicks();

bool inputSneak();
bool inputPrimary();
bool inputSecondary();
bool inputTertiary();
