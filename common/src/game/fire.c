#include "fire.h"

#include "../mods/api_v1.h"
#include "../../../common/src/network/messages.h"

fire fireList[1<<16];
uint fireCount = 0;

#include <stdio.h>

void fireSendUpdate(uint c, uint i){
	fire *f = &fireList[i];
	msgFireUpdate(c,i,fireCount,f->x,f->y,f->z,f->strength);
}

void fireBox(int x, int y, int z, int w, int h, int d){
	for(int cx = x;cx < x+w;cx++){
		for(int cy = y;cy<y+h;cy++){
			for(int cz = z;cz<z+d;cz++){
				const u8 b = worldGetB(cx,cy,cz);
				if(b == 0){continue;}
				fireNew(cx,cy,cz,128);
			}
		}
	}
}

void fireBoxExtinguish(int x, int y, int z, int w, int h, int d){
	if(isClient){return;}
	for(int cx = x;cx < x+w;cx++){
		for(int cy = y;cy<y+h;cy++){
			for(int cz = z;cz<z+d;cz++){
				fire *f = fireGetAtPos(cx,cy,cz);
				if(f == NULL){return;}
				f->strength = MAX(-128,f->strength - 128);
				fireSendUpdate(-1, f - fireList);
				printf("Extinguish I:%u S:%i\n",(int)(f-fireList),f->strength);
			}
		}
	}
}

fire *fireGetAtPos(u16 x,u16 y, u16 z){
	for(uint i=0;i<fireCount;i++){
		fire *f = &fireList[i];
		if(f->x != x){continue;}
		if(f->y != y){continue;}
		if(f->z != z){continue;}
		return f;
	}
	return NULL;
}
