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

#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/side.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

blockId blockTypeMax = 0;

void blockTypeGenMeshes();
void blockTypeSetTex(blockId b, side side, u32 tex);

static void blockTypeInitBlock(blockId b, u32 tex, blockCategory ncat,const char *bname,int nhp, int nfirehp, float weight, u32 ncolor1,u32 ncolor2, u16 ingressMask, u16 egressMask){
	for(side i=0;i<sideMAX;i++){
		blockTypeSetTex(b,i,tex);
	}
	blocks[b].name        = (char *)bname;
	blocks[b].hp          = nhp;
	blocks[b].firehp      = nfirehp;
	blocks[b].cat         = ncat;
	blocks[b].color[0]    = ncolor1;
	blocks[b].color[1]    = ncolor2;
	blocks[b].weight      = weight;
	blocks[b].ingressMask = ingressMask;
	blocks[b].egressMask  = egressMask;
	blocks[b].lightEmission = 0;
	blockTypeMax = MAX(blockTypeMax, b);

	lispDefineID("i-",bname,b);
}

const char *blockTypeGetName(blockId b){
	return blocks[b].name;
}

static void blockTypeSetLightEmission(blockId b, u8 le){
	blocks[b].lightEmission = le;
}

int blockTypeGetHealth(blockId b){
	return blocks[b].hp;
}
blockCategory blockTypeGetCat(blockId b){
	return blocks[b].cat;
}

int blockTypeGetFireHealth(blockId b){
	return blocks[b].firehp;
}
int blockTypeGetFireDamage(blockId b){
	if(b == I_Grass){return 2;}
	if(b == I_Coal) {return 2;}

	switch(blockTypeGetCat(b)){
	default:
		return 0;
	case WOOD:
		return 2;
	case LEAVES:
		return 3;
	}
}

float blockTypeGetWeight(blockId b){
	return blocks[b].weight;
}

u16 blockTypeGetTexX(blockId b, side cside){
	return blocks[b].texX[cside];
}

u16 blockTypeGetTexY(blockId b, side cside){
	return blocks[b].texY[cside];
}

mesh *blockTypeGetMesh(blockId b){
	return blocks[b].singleBlock;
}

u32 blockTypeGetParticleColor(blockId b) {
	if(b==0){return 0xFFFF00FF;}
	return blocks[b].color[rngValR()&1];
}

bool blockTypeValid(blockId b){
	return blocks[b].name != NULL;
}

void blockTypeInit(){
	blockTypeInitBlock    ( 1, 1, DIRT,  "Dirt", 500, 100, 1.5f, 0xFF0A234F,0xFF051B45, 0xF, 0x3FF);
	blockTypeInitBlock    ( 2, 16, DIRT,  "Grass", 240,  40, 1.7f, 0xFF004227,0xFF051B45, 0x1F, 0x3FF);
	blockTypeSetTex       ( 2,sideTop, 0);
	blockTypeSetTex       ( 2,sideBottom, 1);

	blockTypeInitBlock    ( 3, 2, STONE, "Stone", 1200, 255, 5.f, 0xFF5E5E5E,0xFF484848, 0xFFFF, 0xFFF);
	blockTypeInitBlock    ( 4, 3, STONE, "Coal", 800, 250, 4.f, 0xFF262626,0xFF101010, 0xFFFF, 0xFFF);
	blockTypeInitBlock    ( 5, 4, WOOD,  "Spruce Log", 500, 80, 3.f, 0xFF051B25,0xFF07161D, 0xFFFF, 0xFFFF);
	blockTypeInitBlock    ( 6, 5, LEAVES,"Spruce Leafes",  80,  40, 0.2f, 0xFF012C12,0xFF01250F, 0, 0x1F);
	blockTypeInitBlock    ( 7, 7, WOOD,  "Roots", 480,  40, 1.5f, 0xFF14323E,0xFF0D2029, 0x7, 0xFFFF);
	blockTypeInitBlock    ( 8, 35, LEAVES,"Dry Grass", 240, 80, 1.6f, 0xFF11644B,0xFF007552, 0x1F, 0x3FF);
	blockTypeSetTex       ( 8,sideTop, 6);
	blockTypeSetTex       ( 8,sideBottom, 1);

	blockTypeInitBlock    ( 9, 8, STONE, "Obsidian",      2000, 250, 10.f, 0xFF222222,0xFF171717, 0xFFFF, 0xFFF);
	blockTypeInitBlock    (10, 9, WOOD,  "Oak Log",        600,  80, 3.f, 0xFF082C3C,0xFF08242E, 0xFFFF, 0xFFFF);
	blockTypeInitBlock    (11,10, LEAVES,"Oak Leaves",     70,  44, 0.2f, 0xFF004227,0xFF003318, 0, 0x1F);
	blockTypeInitBlock    (12,12, STONE, "Marble Block",  1600, 255, 8.f, 0xFFF0F0F0,0xFFEBEBEB, 0xFFFF, 0xFFF);
	blockTypeInitBlock    (13,11, STONE, "Hematite Ore",  1100, 160, 6.f, 0xFF5B5B72,0xFF5E5E5E, 0xFFFF, 0xFFF);

	blockTypeInitBlock    (14,13, STONE, "Marble Pillar", 1600, 250, 8.f, 0xFFF0F0F0,0xFFEBEBEB, 0xFFFF, 0xFFF);
	blockTypeSetTex       (14,sideTop,12);
	blockTypeSetTex       (14,sideBottom,12);

	blockTypeInitBlock    (15,14, STONE, "Marble Blocks",1600, 250, 8.f,   0xFFF0F0F0,0xFFEBEBEB, 0xFFFF, 0xFFF);
	blockTypeInitBlock    (16,24, LEAVES,"Acacia Leafes",  70,  60, 0.2f,  0xFF003002,0xFF1c6f32, 0, 0x1F);
	blockTypeInitBlock    (17,17, WOOD,  "Boards",        400, 100, 2.5f,  0xFF09678f,0xFF1380af, 0x1F, 0xFFF);
	blockTypeInitBlock    (18,18, STONE, "Crystals",     2500, 250, 2.f,   0xFF997CE8,0xFF4D25B5, 0xFFFF, 0xFFF);
	blockTypeInitBlock    (19,19, LEAVES,"Sakura Leafes",  70,  42, 0.2f,  0xFF997CE8,0xFF4D25B5, 0, 0x1F);
	blockTypeInitBlock    (20,20, WOOD,  "Birch Log",     500,  80, 3.f,   0xFF525255,0xFF525555, 0xFFFF, 0xFFFF);
	blockTypeInitBlock    (21,21, LEAVES,"Flower Bush",    90,  64, 0.3f,  0xFF004227,0xFF003318, 0, 0x1F);
	blockTypeInitBlock    (22,23, LEAVES,"Date Bush",      80,  84, 0.25f, 0xFF00334f,0xFF128394, 0, 0x1F);

	blockTypeInitBlock    (23,26, DIRT,  "Snow Dirt",  500, 200, 1.5f,           0xFF0A234F,0xFF051B45, 0xF, 0x3FF);
	blockTypeInitBlock    (24,25, DIRT,  "Snow Grass", 240, 100, 1.7f,           0xFF004227,0xFF051B45, 0x1F, 0x3FF);
	blockTypeInitBlock    (25,27, DIRT,  "Snowy Spruce Leafes",  60, 80, 0.3f,   0xFF012C12,0xFF01250F, 0, 0x1F);
	blockTypeInitBlock    (26,28, DIRT,  "Snowy Oak Leaves",     70, 80, 0.3f,   0xFF004227,0xFF003318, 0, 0x1F);
	blockTypeInitBlock    (27,29, DIRT,  "Snowy Flower Bush",    90, 124, 0.4f,  0xFF004227,0xFF003318, 0, 0x1F);
	blockTypeInitBlock    (28,30, DIRT,  "Snowy Date Bush",      80, 164, 0.35f, 0xFF00334f,0xFF128394, 0, 0x1F);
	blockTypeInitBlock    (29,31, DIRT,  "Snowy Acacia Leafes",  70, 120, 0.3f,  0xFF003002,0xFF1c6f32, 0, 0x1F);
	blockTypeInitBlock    (30,32, WOOD,  "Snowy Roots", 480,  68, 1.8f,          0xFF14323E,0xFF0D2029, 7, 0xFFFF);
	blockTypeInitBlock    (31,33, DIRT,  "Snowy Sakura Leafes", 70,  82, 0.2f,   0xFF997CE8,0xFF4D25B5, 0, 0x1F);

	blockTypeSetLightEmission(15, 15);

	blockTypeGenMeshes();
}

u16 blockTypeGetIngressMask(blockId b){
	return blocks[b].ingressMask;
}

u16 blockTypeGetEgressMask(blockId b){
	return blocks[b].egressMask;
}

u8 blockTypeGetLightEmission(blockId b){
	return blocks[b].lightEmission;
}
