#include "being.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"

beingList *beingListGet(u16 x, u16 y, u16 z){
	chunk *chnk = worldGetChunk(x,y,z);
	if(chnk != NULL){return &chnk->bl;}
	chungus *chng = worldTryChungus(x>>8,y>>8,z>>8);
	if(chng != NULL){return &chng->bl;}
	return NULL;
}
