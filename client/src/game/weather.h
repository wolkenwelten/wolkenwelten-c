#pragma once
#include "../../../common/src/game/weather.h"

extern u32 cloudCT[128];
extern u32 cloudCB[128];

void cloudsRender    ();
void cloudsDraw      (int cx, int cy, int cz);
void cloudsCalcColors();
void cloudsInitGfx   ();

void rainDrawAll();
void rainRecvUpdate(const packet *p);
