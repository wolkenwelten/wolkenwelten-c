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
#include "light.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/chunkOverlay.h"
#include "../../../common/src/game/light.h"
#include "../../../common/src/game/time.h"
#include "../../../common/src/misc/profiling.h"

void lightGen(chunk *c){
	if((c == NULL) || ((c->flags & CHUNK_FLAG_LIGHT_DIRTY) == 0)){return;}
	const chunkOverlay *chunks[3][3][3];
	if(c->light == NULL){c->light = chunkOverlayAllocate();}
	const int cx = c->x;
	const int cy = c->y;
	const int cz = c->z;

	for(int x=-1;x<2;x++){
	for(int y=-1;y<2;y++){
	for(int z=-1;z<2;z++){
		chunk *chnk = worldTryChunk(cx+(x*CHUNK_SIZE),cy+(y*CHUNK_SIZE),cz+(z*CHUNK_SIZE));
		chunks[x+1][y+1][z+1] = chnk ? chnk->block : NULL;
	}
	}
	}

	lightTick(c->light, chunks);
	c->flags &= ~CHUNK_FLAG_LIGHT_DIRTY;
}

void lightCheckTime(){
	static u8 lastBrightness = 255;
	const u8 newBrightness = gtimeGetBlockBrightness(gtimeGetTimeOfDay());
	if(newBrightness == lastBrightness){return;}
	lastBrightness = newBrightness;
	chunkDirtyAll();
}

float lightAtPos(const vec pos){
	const u8 light = worldTryLight(pos.x, pos.y, pos.z);
	const float rawBrightness = (light * (1.f / 16.f));
	return rawBrightness * rawBrightness;
}
