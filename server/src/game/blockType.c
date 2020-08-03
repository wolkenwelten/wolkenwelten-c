#include "blockType.h"

#include "../../../common/src/misc/misc.h"

#include <string.h>

typedef struct {
	int hp;
	blockCategory cat;
	char *name;
} blockType;

blockType blocks[256];

void blockTypeInitBlock(uint8_t b, uint8_t tex, blockCategory ncat,const char *bname,int nhp, uint32_t c1, uint32_t c2){
	blocks[b].name = (char *)bname;
	blocks[b].hp   = nhp;
	blocks[b].cat  = ncat;

	(void)tex;
	(void)c1;
	(void)c2;
}

void blockTypeSetTex(uint8_t b, int side, uint32_t tex){
	(void)b;
	(void)side;
	(void)tex;
}

const char *blockTypeGetName(uint8_t b){
	return blocks[b].name;
}

int blockTypeGetHP(uint8_t b){
	return blocks[b].hp;
}
blockCategory blockTypeGetCat(uint8_t b){
	return blocks[b].cat;
}

bool blockTypeValid(uint8_t b){
	return blocks[b].name != NULL;
}

void blockTypeInit(){
	#include "../../../common/data/blockType.h"
}
