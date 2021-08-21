#pragma once
#include "../../../common/src/common.h"

extern  int GLVersionMajor;
extern  int GLVersionMinor;
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
void setWindowed(int width, int height, int x, int y);

int getWindowX();
int getWindowY();

void swapWindow();
void setRelativeMouseMode(bool ra);
void warpMouse(int nMouseX,int nMouseY);
uint getMouseState(int *mx, int *my);
uint getTicks();

bool inputSneak();
bool inputBoost();
bool inputPrimary();
bool inputSecondary();
bool inputTertiary();
bool inputThrow();
bool gameControlsInactive();
