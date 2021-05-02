#include "nujel.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/lisp.h"
#include "../../../common/src/nujel/nujel.h"
#include "island.h"

void worldgenNujel(worldgen *wgen){
	if(wgen == NULL){return;}
	lispCallFuncIII("worldgen",wgen->gx,wgen->gy,wgen->gz);
	lClosureGC();

	wgen->minX = wgen->minY = wgen->minZ = 0;
	wgen->maxX = wgen->maxY = wgen->maxZ = 255;

	worldgenRemoveDirt(wgen);
}
