#include "fire.h"

#include "../mods/api_v1.h"
#include "../../../common/src/network/messages.h"

fire fireList[1<<16];
uint fireCount = 0;

void fireSendUpdate(uint c, uint i){
	fire *f = &fireList[i];
	msgFireUpdate(c,i,fireCount,f->x,f->y,f->z,f->strength);
}

void fireBox(int x, int y, int z, int w, int h, int d){
	for(int cx = x;cx < x+w;cx++){
		for(int cy = y;cy<y+h;cy++){
			for(int cz = z;cz<z+d;cz++){
				const vec pos = vecNew(cx,cy,cz);
				if(!vecInWorld(pos)){continue;}
				const u8 b = worldGetB(cx,cy,cz);
				if(b == 0){continue;}

				fireNew(pos,256);
			}
		}
	}
}
