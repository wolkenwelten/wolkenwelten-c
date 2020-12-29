#include "../game/blockType.h"

#include "../../../common/src/game/item.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"

#include <stddef.h>
#include <string.h>

void blockTypeGenMeshes();
void blockTypeSetTex(u8 b, int side, u32 tex);

static void blockTypeInitBlock(u8 b, u32 tex, blockCategory ncat,const char *bname,int nhp, int nfirehp,u32 ncolor1,u32 ncolor2){
	for(int i=0;i<6;i++){
		blockTypeSetTex(b,i,tex);
	}
	blocks[b].name     = (char *)bname;
	blocks[b].hp       = nhp;
	blocks[b].firehp   = nfirehp;
	blocks[b].cat      = ncat;
	blocks[b].color[0] = ncolor1;
	blocks[b].color[1] = ncolor2;
}

static void blockTypeSetWaterProps(u8 b, int waterCapacity, int waterIngress, int waterEgress){
	blocks[b].waterCapacity = waterCapacity;
	blocks[b].waterIngress  = waterIngress;
	blocks[b].waterEgress   = waterEgress;
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

int blockTypeGetFireHP(u8 b){
	return blocks[b].firehp;
}
int blockTypeGetFireDmg(u8 b){
	if(b == I_Grass){return 4;}
	if(b == I_Coal) {return 4;}

	switch(blockTypeGetCat(b)){
	default:
		return 1;
	case WOOD:
		return 4;
	case LEAVES:
		return 6;
	}
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
	if(b==0){return 0xFFFF00FF;}
	return blocks[b].color[rngValR()&1];
}

bool blockTypeValid(u8 b){
	return blocks[b].name != NULL;
}

void blockTypeInit(){
	blockTypeSetWaterProps( 0, (1<<15)-1, (1<<15)-1, (1<<15)-1);

	blockTypeInitBlock    ( 1, 1, DIRT,  "Dirt", 200, 2000, 0xFF0A234F,0xFF051B45);
	blockTypeSetWaterProps( 1, 8192, 32, 8);

	blockTypeInitBlock    ( 2, 16, DIRT,  "Grass", 240,  400, 0xFF004227,0xFF051B45);
	blockTypeSetWaterProps( 2, 8192, 24, 8);
	blockTypeSetTex       ( 2, 2, 0);
	blockTypeSetTex       ( 2, 3, 1);

	blockTypeInitBlock    ( 3, 2, STONE, "Stone", 1000, 4000, 0xFF5E5E5E,0xFF484848);
	blockTypeSetWaterProps( 3, 0, 0, 8);

	blockTypeInitBlock    ( 4, 3, STONE, "Coal", 800, 4000, 0xFF262626,0xFF101010);
	blockTypeSetWaterProps( 4, 0, 0, 8);

	blockTypeInitBlock    ( 5, 4, WOOD,  "Spruce Log", 500,  800, 0xFF051B25,0xFF07161D);
	blockTypeSetWaterProps( 5, 2048, 0, 8);

	blockTypeInitBlock    ( 6, 5, LEAVES,"Spruce Leafes",  110,  400, 0xFF012C12,0xFF01250F);
	blockTypeSetWaterProps( 6, 128, 128, 128);

	blockTypeInitBlock    ( 7, 7, WOOD,  "Roots", 480,  480, 0xFF14323E,0xFF0D2029);
	blockTypeSetWaterProps( 7, 8192, 128, 8);

	blockTypeInitBlock    ( 2, 16, DIRT,  "Grass", 240,  400, 0xFF004227,0xFF051B45);
	blockTypeSetWaterProps( 2, 8192, 24, 8);
	blockTypeSetTex       ( 2, 2, 0);
	blockTypeSetTex       ( 2, 3, 1);

	blockTypeInitBlock    ( 8, 22, LEAVES,  "Dry Grass", 240, 1000, 0xFF11644B,0xFF007552);
	blockTypeSetWaterProps( 8, 8192, 24, 8);
	blockTypeSetTex       ( 8, 2, 6);
	blockTypeSetTex       ( 8, 3, 1);

	blockTypeInitBlock    ( 9, 8, STONE, "Obsidian",      2000, 8000, 0xFF222222,0xFF171717);
	blockTypeSetWaterProps( 9, 0, 0, 8);

	blockTypeInitBlock    (10, 9, WOOD,  "Oak Log",        500,  800, 0xFF082C3C,0xFF08242E);
	blockTypeSetWaterProps(10, 2048, 0, 8);

	blockTypeInitBlock    (11,10, LEAVES,"Oak Leaves",     100,  440, 0xFF004227,0xFF003318);
	blockTypeSetWaterProps(11, 128, 128, 128);

	blockTypeInitBlock    (12,12, STONE, "Marble Block",  1600, 8000, 0xFFF0F0F0,0xFFEBEBEB);
	blockTypeSetWaterProps(12, 0, 0, 8);

	blockTypeInitBlock    (13,11, STONE, "Hematite Ore",  1100, 4000, 0xFF5B5B72,0xFF5E5E5E);
	blockTypeSetWaterProps(13, 0, 0, 8);

	blockTypeInitBlock    (14,13, STONE, "Marble Pillar", 1600, 8000, 0xFFF0F0F0,0xFFEBEBEB);
	blockTypeSetWaterProps(14, 0, 0, 8);
	blockTypeSetTex       (14,2,12);
	blockTypeSetTex       (14,3,12);

	blockTypeInitBlock    (15,14, STONE, "Marble Blocks", 1600, 8000, 0xFFF0F0F0,0xFFEBEBEB);
	blockTypeSetWaterProps(15, 0, 0, 8);

	// 16 Unused

	blockTypeInitBlock    (17,17, WOOD,  "Boards",         300,  600, 0xFF09678f,0xFF1380af);
	blockTypeSetWaterProps(17, 0, 0, 8);

	blockTypeInitBlock    (18,18, STONE, "Crystals",      2500, 8000, 0xFF997CE8,0xFF4D25B5);
	blockTypeSetWaterProps(18, 0, 0, 8);

	blockTypeInitBlock    (19,19, LEAVES,"Sakura Leafes",  120,  420, 0xFF997CE8,0xFF4D25B5);
	blockTypeSetWaterProps(19, 128, 128, 128);

	blockTypeInitBlock    (20,20, WOOD,  "Birch Log",      500,  800, 0xFF525255,0xFF525555);
	blockTypeSetWaterProps(20, 2048, 0, 8);

	blockTypeInitBlock    (21,21, LEAVES,"Flower Bush",    160,  440, 0xFF004227,0xFF003318);
	blockTypeSetWaterProps(21, 128, 128, 128);

	blockTypeGenMeshes();
}
