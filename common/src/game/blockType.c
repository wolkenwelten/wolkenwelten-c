#include "../game/blockType.h"

#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"

void blockTypeGenMeshes();
void blockTypeSetTex(u8 b, int side, u32 tex);

void blockTypeInitBlock(u8 b, u32 tex, blockCategory ncat,const char *bname,int nhp,u32 ncolor1,u32 ncolor2){
	for(int i=0;i<6;i++){
		blockTypeSetTex(b,i,tex);
	}
	blocks[b].name = (char *)bname;
	blocks[b].hp = nhp;
	blocks[b].cat = ncat;
	blocks[b].color[0] = ncolor1;
	blocks[b].color[1] = ncolor2;
}

const char *blockTypeGetName(u8 b){
	return blocks[b].name;
}

int blockTypeGetHP(u8 b){
	return blocks[b].hp;
}
blockCategory blockTypeGetCat(u8 b){
	return blocks[b].cat;
}

u16 blockTypeGetTexX(u8 b, int side){
	return blocks[b].texX[side];
}

u16 blockTypeGetTexY(u8 b, int side){
	return blocks[b].texY[side];
}

mesh *blockTypeGetMesh(u8 b){
	return blocks[b].singleBlock;
}

u32 blockTypeGetParticleColor(u8 b) {
	return blocks[b].color[rngValR()&1];
}

bool blockTypeValid(u8 b){
	return blocks[b].name != NULL;
}

void blockTypeInit(){
	blockTypeInitBlock( 1, 1, DIRT,  "Dirt",           200, 0xFF0A234F,0xFF051B45);
	blockTypeInitBlock( 2, 7, DIRT,  "Grass",          240, 0xFF004227,0xFF051B45); blockTypeSetTex(2,2,0); blockTypeSetTex(2,3,1);
	blockTypeInitBlock( 3, 2, STONE, "Stone",         1000, 0xFF5E5E5E,0xFF484848);
	blockTypeInitBlock( 4, 3, STONE, "Coal",           800, 0xFF262626,0xFF101010);
	blockTypeInitBlock( 5, 4, WOOD,  "Spruce Log",     500, 0xFF051B25,0xFF07161D);
	blockTypeInitBlock( 6, 5, LEAVES,"Spruce Leaves",   40, 0xFF012C12,0xFF01250F);
	blockTypeInitBlock( 7, 6, WOOD,  "Roots",          170, 0xFF14323E,0xFF0D2029);
	blockTypeInitBlock( 8, 6, WOOD,  "Dirt Roots",     480, 0xFF14323E,0xFF0D2029);
	blockTypeInitBlock( 9, 8, STONE, "Obsidian",      2000, 0xFF222222,0xFF171717);
	blockTypeInitBlock(10, 9, WOOD,  "Oak Log",        500, 0xFF082C3C,0xFF08242E);
	blockTypeInitBlock(11,10, LEAVES,"Oak Leaves",      40, 0xFF004227,0xFF003318);
	blockTypeInitBlock(12,12, STONE, "Marble Block",  1600, 0xFFF0F0F0,0xFFEBEBEB);
	blockTypeInitBlock(13,11, STONE, "Hematite Ore",  1100, 0xFF5B5B72,0xFF5E5E5E);
	blockTypeInitBlock(14,13, STONE, "Marble Pillar", 1600, 0xFFF0F0F0,0xFFEBEBEB); blockTypeSetTex(14,2,12); blockTypeSetTex(14,3,12);
	blockTypeInitBlock(15,14, STONE, "Marble Blocks", 1600, 0xFFF0F0F0,0xFFEBEBEB);
	blockTypeInitBlock(16,16, WOOD,  "Hewn Log",       400, 0xFF09678f,0xFF1380af); blockTypeSetTex(16,2,15); blockTypeSetTex(16,3,15);
	blockTypeInitBlock(17,17, WOOD,  "Boards",         300, 0xFF09678f,0xFF1380af);
	blockTypeInitBlock(18,18, STONE, "Crystals",      2500, 0xFF997CE8,0xFF4D25B5);
	blockTypeInitBlock(19,19, WOOD,  "Sakura Leaves",   50, 0xFF997CE8,0xFF4D25B5);
	blockTypeInitBlock(20,20, WOOD,  "Birch Log",      500, 0xFF525255,0xFF525555);
	blockTypeInitBlock(21,21, LEAVES,"Flower Bush",     60, 0xFF004227,0xFF003318);

	blockTypeGenMeshes();
}
