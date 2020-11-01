#pragma once
#include "../common.h"

vec   beingGetPos    (being b);
void  beingSetPos    (being b, const vec pos);
void  beingAddPos    (being b, const vec pos);

vec   beingGetVel    (being b);
void  beingSetVel    (being b, const vec vel);
void  beingAddVel    (being b, const vec vel);

float beingGetWeight (being b);

being beingClosest   (const vec pos, float maxDistance);
