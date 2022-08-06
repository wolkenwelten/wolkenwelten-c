#pragma once
#include "../common.h"

#define PROFILE_START() static uint _profIndex = 0; \
	if (_profIndex == 0) { \
		_profIndex = profGetIndex(__FUNCTION__); \
	 } \
	profStart(_profIndex)

#define PROFILE_STOP() profStop(_profIndex)

uint        profGetIndex  (const char *funcName);
void        profStart     (uint i);
void        profStop      (uint i);
u64         profGetTotal  (uint i);
double      profGetMean   (uint i);
double      profGetShare  (uint i);
const char *profGetName   (uint i);
const char *profGetReport ();
void        profReset     ();


void        nprofAddPacket (uint type, uint size);
double      nprofGetShare  (uint type);
const char *nprofGetReport ();
void        nprofReset     ();

void profileStartupTime(const char *msg);
