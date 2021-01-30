/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Most of this code is directly from Mark Morley's Tutorials
 * which can be found here:
 * http://www.crownandcutlass.com/features/technicaldetails/frustum.html
 */
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

bool pointInFrustum(const vec pos){
	for(int p = 0; p < 6; p++ ){
		if( frustum[p][0] * pos.x + frustum[p][1] * pos.y + frustum[p][2] * pos.z + frustum[p][3] <= 0 ){
			return false;
		}
	}
	return true;
}

bool CubeInFrustum(const vec pos, float size ){
	const float x = pos.x;
	const float y = pos.y;
	const float z = pos.z;

	for(int p = 0; p < 6; p++ ){
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
