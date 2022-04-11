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
#include "snow.h"
#include "weather.h"

#include "../../game/blockType.h"
#include "../../game/fire.h"
#include "../../misc/profiling.h"
#include "../../network/packet.h"
#include "../../network/messages.h"
#include "../../world/world.h"

__attribute__((aligned(32)))   snowDrop glSnowDrops[SNOW_MAX];
__attribute__((aligned(32)))   snowDrop   snowDrops[SNOW_MAX];
__attribute__((aligned(32)))      float   snowVel[8];
                                    u64   snowCoords[SNOW_MAX];
uint snowCount = 0;
u8 snowIntensity = 0;

void fxSnowDrop(const vec pos);

void snowInit(){
	snowIntensity = 0;
}

static void snowDel(uint i){
	glSnowDrops[i]  = glSnowDrops[--snowCount];
	  snowDrops[i]  =   snowDrops[  snowCount];
	  snowCoords[i] =   snowCoords[ snowCount];
}

void snowPosUpdatePortable(){
	for(uint i=0;i<snowCount;i++){
		glSnowDrops[i].x     += snowDrops[i].x;
		glSnowDrops[i].y     += snowDrops[i].y;
		glSnowDrops[i].z     += snowDrops[i].z;
		glSnowDrops[i].size  += snowDrops[i].size;

		  snowDrops[i].x     += snowVel[0];
		  snowDrops[i].y     += snowVel[1];
		  snowDrops[i].z     += snowVel[2];
		  snowDrops[i].size  += snowVel[3];

		  snowDrops[i].x     += snowDrops[i].z * 0.01f;
		  snowDrops[i].z     -= snowDrops[i].x * 0.01f;
	}
}

void snowBox(u16 x, u16 y, u16 z, u16 w, u16 h, u16 d){
	for(int cx=x;cx<x+w;cx++){
	for(int cy=y;cy<y+h;cy++){
	for(int cz=z;cz<z+d;cz++){
		const u8 b = worldGetB(cx,cy,cz);
		switch(b){
		case I_Spruce_Leaf:
			worldSetB(cx,cy,cz,I_Snowy_Spruce_Leaf);
			break;
		case I_Oak_Leaf:
			worldSetB(cx,cy,cz,I_Snowy_Oak_Leaf);
			break;
		case I_Flower:
			worldSetB(cx,cy,cz,I_Snowy_Flower);
			break;
		case I_Date:
			worldSetB(cx,cy,cz,I_Snowy_Date);
			break;
		case I_Acacia_Leaf:
			worldSetB(cx,cy,cz,I_Snowy_Acacia_Leaf);
			break;
		case I_Roots:
			worldSetB(cx,cy,cz,I_Snowy_Roots);
			break;
		case I_Sakura_Leaf:
			worldSetB(cx,cy,cz,I_Snowy_Sakura_Leaf);
			break;
		case I_Grass:
		case I_Dry_Grass:
			worldSetB(cx,cy,cz,I_Snow_Grass);
			break;
		case I_Dirt:
			worldSetB(cx,cy,cz,I_Snow_Dirt);
			break;
		}
	}
	}
	}
}

void snowUpdateAll(){
	PROFILE_START();

	snowVel[0] = windVel.x / 8.f;
	snowVel[1] = -0.0000001;
	snowVel[2] = windVel.z / 8.f;
	snowVel[3] = 0.f;

	for(uint i=0;i<4;i++){
		snowVel[i+4] = snowVel[i];
	}

	snowPosUpdatePortable();
	for(uint i=snowCount-1;i<snowCount;i--){
		const snowDrop *glrd = &glSnowDrops[i];
		if((glrd->y < 0.f) || (glrd->size < 0.f)){
			snowDel(i);
			continue;
		}
		const u64 newCoords = ((u64)glrd->x & 0xFFFF) | (((u64)glrd->y & 0xFFFF) << 16) | (((u64)glrd->z & 0xFFFF) << 32);
		if(newCoords != snowCoords[i]){
			if(!worldIsLoaded(glrd->x,glrd->y,glrd->z)){
				snowDel(i);
				continue;
			}
			if(worldGetB(glrd->x,glrd->y,glrd->z) != 0){
				if(isClient){
					fxSnowDrop(vecNew(glrd->x,glrd->y,glrd->z));
				}else{
					snowBox(glrd->x-1, glrd->y-1, glrd->z-1, 3, 3, 3);
				}
				snowDel(i);
				continue;
			}
			snowCoords[i] = newCoords;
		}
	}
	PROFILE_STOP();
}

void snowNew(vec pos){
	uint i = ++snowCount;
	if(i >= SNOW_MAX){i = rngValA(SNOW_MAX-1); snowCount--;}

	glSnowDrops[i]  = (snowDrop){ pos.x, pos.y, pos.z, 256.f };
	  snowDrops[i]  = (snowDrop){ windVel.x, -0.1f, windVel.z, -0.02f };
          snowCoords[i] = 0;

	if(!isClient){snowSendUpdate(-1,i);}
}

void snowSendUpdate(uint c, uint i){
	packet *p = &packetBuffer;

	p->v.f[0] = glSnowDrops[i].x;
	p->v.f[1] = glSnowDrops[i].y;
	p->v.f[2] = glSnowDrops[i].z;

	packetQueue(p,msgtSnowRecvUpdate,3*4,c);
}

void weatherSetSnowIntensity(u8 intensity){
	snowIntensity = intensity;
	if(!isClient){weatherSendUpdate(-1);}
}
