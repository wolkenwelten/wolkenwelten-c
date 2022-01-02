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
#include "fluid.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/fluid.h"
#include "../../../common/src/misc/profiling.h"


void fluidPhysicsTick(){
	static int calls = 0;
	PROFILE_START();

	for(uint i = calls&0xF; i < chungusCount; i+=0x10){
		chungus *cng = &chungusList[i];
		for(int x=0;x<16;x++){
		for(int y=0;y<16;y++){
		for(int z=0;z<16;z++){
			chunk *c = &cng->chunks[x][y][z];
			if(c->fluid == NULL){continue;}
			fluidPhysics(c->fluid, c->x, c->y, c->z);
		}
		}
		}
	}
	++calls;

	PROFILE_STOP();
}
