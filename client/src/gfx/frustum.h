#pragma once
#include "../../../common/src/common.h"
extern float frustum[6][4];

void extractFrustum();
bool pointInFrustum(const vec pos);
bool CubeInFrustum (const vec pos, float size );
