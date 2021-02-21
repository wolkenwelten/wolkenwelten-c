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

#include "../game/blockType.h"

#include "../../../common/src/game/item.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/side.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

void blockTypeGenMeshes();
void blockTypeSetTex(u8 b, side side, u32 tex);

static void blockTypeInitBlock(u8 b, u32 tex, blockCategory ncat,const char *bname,int nhp, int nfirehp,u32 ncolor1,u32 ncolor2){

	for(side i=0;i<sideMAX;i++){
		blockTypeSetTex(b,i,tex);
	}
	blocks[b].name     = (char *)bname;
	blocks[b].hp       = nhp;
	blocks[b].firehp   = nfirehp;
	blocks[b].cat      = ncat;
	blocks[b].color[0] = ncolor1;
	blocks[b].color[1] = ncolor2;

	lispDefineID("i-",bname,b);
}

static void blockTypeSetWaterProps(u8 b, int waterCapacity, int waterIngress, int waterEgress){
	blocks[b].waterCapacity = waterCapacity;
	blocks[b].waterIngress  = waterIngress;
	blocks[b].waterEgress   = waterEgress;
}

const char *blockTypeGetName(u8 b){
	return blocks[b].name;
}

int blockTypeGetHealth(u8 b){
	return blocks[b].hp;
}
blockCategory blockTypeGetCat(u8 b){
	return blocks[b].cat;
}

int blockTypeGetFireHealth(u8 b){
	return blocks[b].firehp;
}
int blockTypeGetFireDamage(u8 b){
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

u16 blockTypeGetTexX(u8 b, side cside){
	return blocks[b].texX[cside];
}

u16 blockTypeGetTexY(u8 b, side cside){
	return blocks[b].texY[cside];
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
	blockTypeSetTex       ( 2, sideTop, 0);
	blockTypeSetTex       ( 2, sideBottom, 1);

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
	blockTypeSetTex       ( 2, sideTop, 0);
	blockTypeSetTex       ( 2, sideBottom, 1);

	blockTypeInitBlock    ( 8, 22, LEAVES,  "Dry Grass", 240, 1000, 0xFF11644B,0xFF007552);
	blockTypeSetWaterProps( 8, 8192, 24, 8);
	blockTypeSetTex       ( 8, sideTop, 6);
	blockTypeSetTex       ( 8, sideBottom, 1);

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
	blockTypeSetTex       (14,sideTop,12);
	blockTypeSetTex       (14,sideBottom,12);

	blockTypeInitBlock    (15,14, STONE, "Marble Blocks", 1600, 8000, 0xFFF0F0F0,0xFFEBEBEB);
	blockTypeSetWaterProps(15, 0, 0, 8);

	blockTypeInitBlock    (16,24, LEAVES,"Acacia Leafes",  110,  600, 0xFF000230,0xFF1c638f);
	blockTypeSetWaterProps(16, 128, 128, 128);

	blockTypeInitBlock    (17,17, WOOD,  "Boards",         300,  600, 0xFF09678f,0xFF1380af);
	blockTypeSetWaterProps(17, 0, 0, 8);

	blockTypeInitBlock    (18,18, STONE, "Crystals",      2500, 8000, 0xFF997CE8,0xFF4D25B5);
	blockTypeSetWaterProps(18, 0, 0, 8);

	blockTypeInitBlock    (19,19, LEAVES,"Sakura Leafes",  120,  420, 0xFF997CE8,0xFF4D25B5);
	blockTypeSetWaterProps(19, 128, 128, 128);

	blockTypeInitBlock    (20,20, WOOD,  "Birch Log",      500,  800, 0xFF525255,0xFF525555);
	blockTypeSetWaterProps(20, 2048, 0, 8);

	blockTypeInitBlock    (21,21, LEAVES,"Flower Bush",    160,  640, 0xFF004227,0xFF003318);
	blockTypeSetWaterProps(21, 128, 128, 128);

	blockTypeInitBlock    (22,23, LEAVES,"Date Bush",      120,  840, 0xFF00334f,0xFF128394);
	blockTypeSetWaterProps(22, 128, 128, 128);

	blockTypeGenMeshes();
}
