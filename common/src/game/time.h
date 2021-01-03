#pragma once
#include "../common.h"

#define TIME_NIGHT   0
#define TIME_MORNING 1
#define TIME_NOON    2
#define TIME_EVENING 3

void        gtimeSetTime        (uint newTime);
void        gtimeSetTimeOfDay   (uint newTime);
void        gtimeSetTimeOfDayHR (uint newHours, uint newMinutes);
void        gtimeSetTimeOfDayHRS(const char *newTime);
uint        gtimeGetTime        ();
uint        gtimeGetTimeOfDay   ();
uint        gtimeGetTimeCat     ();
const char *gtimeGetTimeOfDayHRS(uint timeCur);
float       gtimeGetBrightness  (uint time);

void printDebugtime();

void gtimeUpdate();
