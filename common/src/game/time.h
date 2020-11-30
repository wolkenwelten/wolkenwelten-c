#pragma once
#include "../common.h"

void        gtimeSetTime        (uint newTime);
void        gtimeSetTimeOfDay   (uint newTime);
void        gtimeSetTimeOfDayHR (uint newHours, uint newMinutes);
void        gtimeSetTimeOfDayHRS(const char *newTime);
uint        gtimeGetTime        ();
uint        gtimeGetTimeOfDay   ();
const char *gtimeGetTimeOfDayHRS(uint timeCur);

void gtimeUpdate();
