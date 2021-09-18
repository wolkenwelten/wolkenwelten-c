#include "nujel.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/lisp.h"
#include "../../../common/nujel/lib/api.h"
#include "island.h"

#include <stdio.h>

void worldgenNujel(worldgen *wgen){
	if(wgen == NULL){return;}

	for(int x=-2;x<=2;x++){
	for(int y=-2;y<=2;y++){
	for(int z=-2;z<=2;z++){
		const int cx = wgen->gx + x * CHUNGUS_SIZE;
		const int cy = wgen->gy + y * CHUNGUS_SIZE;
		const int cz = wgen->gz + z * CHUNGUS_SIZE;
		lispCallFuncIII("worldgen",cx,cy,cz);
		lGarbageCollect();
	}
	}
	}

	wgen->minX = wgen->minY = wgen->minZ =   0;
	wgen->maxX = wgen->maxY = wgen->maxZ = 255;

	worldgenRemoveDirt(wgen);
}
