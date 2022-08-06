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
#include "worldgen.h"

#include "../voxel/chungus.h"
#include "../voxel/chunk.h"

#include <stdlib.h>
#include <math.h>

chungus *worldGenChungus(chungus *chng){
	const int cy = chng->y<<8;
	const int cx = (chng->x<<8);
	const int cz = (chng->z<<8);
	const int cxd = (1<<15) - cx;
	const int czd = (1<<15) - cz;
	const float cd = sqrtf(cxd*cxd + czd*czd);
	if(cy != 256){return chng;}
	if(cd > 600){return chng;}

	for(int x = 0;x<256;x++){
	for(int z = 0;z<256;z++){
		const int xd = (1<<15) - (cx+x);
		const int zd = (1<<15) - (cz+z);
		const float d = xd*xd + zd*zd;
		if(d > (128*128)){continue;}
		const int sy = 8;
		const int h = 8;

		chungusBox(chng, x, MAX(0,sy-h), z, 1, h, 1, 1);
		chungusBox(chng, x, sy, z, 1, 1, 1, 2);
	}
	}

	return chng;
}
