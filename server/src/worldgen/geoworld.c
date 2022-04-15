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

#include "geoworld.h"

#include "worldgen.h"
#include "../voxel/chungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"

#include <stdlib.h>

void worldgenGeoSphere(worldgen *wgen, int x,int y,int z,int size,int b, int fb){
	float rsq      = (size*size);
	float crystalr = rsq / 2.f;
	if(crystalr < 5.f){crystalr = 0.f;}

	for(int cy=-size;cy<=size;cy++){
		for(int cx = -size;cx <= size;cx++){
			for(int cz = -size;cz <= size;cz++){
				const float d = (cx*cx)+(cz*cz)+(cy*cy);
				if(d < crystalr){
					chungusSetB(wgen->clay,x+cx,y+cy,z+cz,fb);
				}else if(d < rsq){
					chungusSetB(wgen->clay,x+cx,y+cy,z+cz,b);
				}
			}
		}
	}
}

void worldgenGeoRoundPrism(worldgen *wgen, int x,int y,int z,int size,int b,int fb){
	for(int cy=-size;cy<=size;cy++){
		int r     = (size-abs(cy))/2;
		float rsq = (r*r)*0.8f;
		float crystalr = rsq / 2.f;
		if(crystalr < 5.f){crystalr = 0.f;}
		for(int cx = -r;cx <= r;cx++){
			for(int cz = -r;cz <= r;cz++){
				const float d = (cx*cx)+(cz*cz);
				if(d < crystalr){
					chungusSetB(wgen->clay,x+cx,y+cy,z+cz,fb);
				} else if(d < rsq){
					chungusSetB(wgen->clay,x+cx,y+cy,z+cz,b);
				}
			}
		}
	}
}

void worldgenGeoPrism(worldgen *wgen, int x,int y,int z,int size,int b,int fb){
	for(int cy=-size;cy<=size;cy++){
		int r  = (size-abs(cy))/2;
		int cr = r / 2;
		if(r < 2){cr = 0;}
		for(int cx = -r;cx <= r;cx++){
			for(int cz = -r;cz <= r;cz++){
				chungusSetB(wgen->clay,x+cx,y+cy,z+cz,b);
			}
		}
		if(cr == 0){continue;}
		for(int cx = -cr;cx <= cr;cx++){
			for(int cz = -cr;cz <= cr;cz++){
				chungusSetB(wgen->clay,x+cx,y+cy,z+cz,fb);
			}
		}
	}
}

void worldgenGeoPyramid(worldgen *wgen, int x,int y,int z,int size,int b,int fb){
	for(int cy=-size;cy<=size;cy++){
		int r  = size-abs(cy);
		int cr = r / 2;
		if(r < 2){cr = 0;}
		for(int cx = -r;cx <= r;cx++){
			for(int cz = -r;cz <= r;cz++){
				chungusSetB(wgen->clay,x+cx,y+cy,z+cz,b);
			}
		}
		if(cr == 0){continue;}
		for(int cx = -cr;cx <= cr;cx++){
			for(int cz = -cr;cz <= cr;cz++){
				chungusSetB(wgen->clay,x+cx,y+cy,z+cz,fb);
			}
		}
	}
}

void worldgenGeoCube(worldgen *wgen, int x, int y, int z, int size, int b, int fb){
	chungusBox(wgen->clay,x,y,z,size,size,size,b);
	if(size > 4){
		chungusBox(wgen->clay,x+size/4,y+size/4,z+size/4,size/2,size/2,size/2,fb);
	}
}

void worldgenGeoIsland(worldgen *wgen, int x,int y,int z,int size){
	int b=12;
	int fb=18;
	switch(rngValM(2)){
		case 0:
			b = 9;
		break;
		default:
		case 1:
			b = 12;
		break;
	}

	x &= 0xF0;
	y &= 0xF0;
	z &= 0xF0;
	if(x == 0){x += 0x10;}
	if(y == 0){y += 0x10;}
	if(z == 0){z += 0x10;}
	if(x == 0xF0){x -= 0x10;}
	if(y == 0xF0){y -= 0x10;}
	if(z == 0xF0){z -= 0x10;}

	switch(rngValM(96)){
		case 0:
			if(size > 6){
				size *= 9;
			}
		break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			if(size > 6){
				size *= 3;
			}
		break;
	}

	switch(rngValM(4)){
		case 0:
			fb=18;
		break;
		case 1:
			fb=13;
		break;
		case 2:
			fb=4;
		break;
		case 3:
			fb=0;
		break;
	}

	switch(rngValM(5)){
		default:
		case 0:
			worldgenGeoCube(wgen,x,y,z,size,b,fb);
		break;
		case 1:
			worldgenGeoPrism(wgen,x,y,z,size,b,fb);
		break;
		case 2:
			worldgenGeoPyramid(wgen,x,y,z,size,b,fb);
		break;
		case 3:
			worldgenGeoRoundPrism(wgen,x,y,z,size,b,fb);
		break;
		case 4:
			worldgenGeoSphere(wgen,x,y,z,size,b,fb);
		break;
	}
}
