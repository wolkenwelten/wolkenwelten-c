#pragma once
#include <stdbool.h>

extern int mousex,mousey,mouseClicked[3];
extern bool mouseHidden;
extern float matOrthoProj[16];

void initUI();
void resizeUI();
void showMouseCursor();
void hideMouseCursor();
void updateMouse();
void drawCursor();
void renderLoadingUI(const char *step);
void renderUI();

void resetOverlayColor();
void commitOverlayColor();
void setOverlayColor(unsigned int color, unsigned int animationDuration);