#include "frustum.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"

float frustum[6][4];

void extractFrustum(){
	float   clip[16];
	matMul(clip,matView,matProjection);

	/* Extract the numbers for the RIGHT plane */
	frustum[0][0] = clip[ 3] - clip[ 0];
	frustum[0][1] = clip[ 7] - clip[ 4];
	frustum[0][2] = clip[11] - clip[ 8];
	frustum[0][3] = clip[15] - clip[12];

	/* Extract the numbers for the LEFT plane */
	frustum[1][0] = clip[ 3] + clip[ 0];
	frustum[1][1] = clip[ 7] + clip[ 4];
	frustum[1][2] = clip[11] + clip[ 8];
	frustum[1][3] = clip[15] + clip[12];

	/* Extract the BOTTOM plane */
	frustum[2][0] = clip[ 3] + clip[ 1];
	frustum[2][1] = clip[ 7] + clip[ 5];
	frustum[2][2] = clip[11] + clip[ 9];
	frustum[2][3] = clip[15] + clip[13];

	/* Extract the TOP plane */
	frustum[3][0] = clip[ 3] - clip[ 1];
	frustum[3][1] = clip[ 7] - clip[ 5];
	frustum[3][2] = clip[11] - clip[ 9];
	frustum[3][3] = clip[15] - clip[13];

	/* Extract the FAR plane */
	frustum[4][0] = clip[ 3] - clip[ 2];
	frustum[4][1] = clip[ 7] - clip[ 6];
	frustum[4][2] = clip[11] - clip[10];
	frustum[4][3] = clip[15] - clip[14];

	/* Extract the NEAR plane */
	frustum[5][0] = clip[ 3] + clip[ 2];
	frustum[5][1] = clip[ 7] + clip[ 6];
	frustum[5][2] = clip[11] + clip[10];
	frustum[5][3] = clip[15] + clip[14];
}

bool pointInFrustum( float x, float y, float z ){
	int p;

	for( p = 0; p < 6; p++ ){
		if( frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3] <= 0 ){
			return false;
		}
	}
	return true;
}

bool CubeInFrustum( float x, float y, float z, float size ){
	int p;

	for( p = 0; p < 6; p++ ){
		if( frustum[p][0] * (x       ) + frustum[p][1] * (y       ) + frustum[p][2] * (z       ) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x + size) + frustum[p][1] * (y       ) + frustum[p][2] * (z       ) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x       ) + frustum[p][1] * (y + size) + frustum[p][2] * (z       ) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z       ) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x       ) + frustum[p][1] * (y       ) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x + size) + frustum[p][1] * (y       ) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x       ) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
			continue;
		if( frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > 0 )
			continue;
		return false;
	}
	return true;
}
