#pragma once
#include <stdbool.h>
extern float frustum[6][4];

void extractFrustum();
bool pointInFrustum( float x, float y, float z );
bool CubeInFrustum( float x, float y, float z, float size );
