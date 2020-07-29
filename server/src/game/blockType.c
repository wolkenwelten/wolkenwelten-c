#include "../game/blockType.h"
#include "../../../common/src/misc.h"

#include <string.h>

typedef struct {
	int hp;
	blockCategory cat;
	char *name;
} blockType;

blockType blocks[256];

void blockTypeInitBlock(uint8_t b, blockCategory ncat,const char *bname,int nhp){
	blocks[b].name = (char *)bname;
	blocks[b].hp   = nhp;
	blocks[b].cat  = ncat;
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
	blockTypeInitBlock( 1, DIRT,  "Dirt",           200);
	blockTypeInitBlock( 2, DIRT,  "Grass",          240);
	blockTypeInitBlock( 3, STONE, "Stone",         1000);
	blockTypeInitBlock( 4, STONE, "Coal",           800);
	blockTypeInitBlock( 5, WOOD,  "Spruce Log",     500);
	blockTypeInitBlock( 6, LEAVES,"Spruce Leaves",   40);
	blockTypeInitBlock( 7, WOOD,  "Roots",          170);
	blockTypeInitBlock( 8, WOOD,  "Dirt Roots",     480);
	blockTypeInitBlock( 9, STONE, "Obsidian",      2000);
	blockTypeInitBlock(10, WOOD,  "Oak Log",        500);
	blockTypeInitBlock(11, LEAVES,"Oak Leaves",      40);
	blockTypeInitBlock(12, STONE, "Marble Block",  1600);
	blockTypeInitBlock(13, STONE, "Hematite Ore",  1100);
	blockTypeInitBlock(14, STONE, "Marble Pillar", 1600);
	blockTypeInitBlock(15, STONE, "Marble Blocks", 1600);
	blockTypeInitBlock(16, WOOD,  "Hewn Log",       400);
	blockTypeInitBlock(17, WOOD,  "Boards",         300);
	blockTypeInitBlock(18, STONE, "Crystals",      2500);
	blockTypeInitBlock(19, WOOD,  "Sakura Leaves",   50);
}
