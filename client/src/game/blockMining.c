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

#include "../game/blockMining.h"

#include "../game/blockType.h"
#include "../gfx/effects.h"
#include "../gfx/gl.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"

#include <math.h>
#include <stdlib.h>

typedef struct {
	u16 x,y,z;
	int damage;
	int lastDamage;
	blockId b;
	u8 wasMined;
} blockMining;

blockMining blockMiningList[512];
int         blockMiningCount = 0;
mesh       *blockMiningProgressMesh;

float blockMiningGetProgress(blockMining *bm){
	if(bm->b == 0){return 0.f;}
	return ((float)bm->damage) / ((float)blockTypeGetHealth(bm->b));
}

void blockMiningBurnBlock(int x, int y, int z, blockId b){
	if(b == 0){return;}
	if((b == I_Grass) || (b == I_Dry_Grass) || (b == I_Roots) || (b == I_Snow_Grass)){
		worldSetB(x,y,z,I_Dirt);
	}else{
		worldSetB(x,y,z,0);
	}
}

int blockMiningUpdateMesh(int d){
	blockMining *bm = &blockMiningList[d];
	const int frame = floorf(blockMiningGetProgress(bm)*8);
	const float TILE = 1.f/8.f;
	const float LoX  = TILE* frame;
	const float HiX  = TILE*(frame+1);
	const float xa = bm->x;
	const float xb = xa+1;
	const float ya = bm->y;
	const float yb = ya+1;
	const float za = bm->z;
	const float zb = za+1;
	mesh *m = blockMiningProgressMesh;

	// Front Face
	meshAddVert(m, xa, ya, zb, LoX,1.0f);
	meshAddVert(m, xb, ya, zb, HiX,1.0f);
	meshAddVert(m, xb, yb, zb, HiX,0.0f);
	meshAddVert(m, xb, yb, zb, HiX,0.0f);
	meshAddVert(m, xa, yb, zb, LoX,0.0f);
	meshAddVert(m, xa, ya, zb, LoX,1.0f);
	// Back Face
	meshAddVert(m, xa, ya, za, HiX,1.0f);
	meshAddVert(m, xa, yb, za, HiX,0.0f);
	meshAddVert(m, xb, yb, za, LoX,0.0f);
	meshAddVert(m, xb, yb, za, LoX,0.0f);
	meshAddVert(m, xb, ya, za, LoX,1.0f);
	meshAddVert(m, xa, ya, za, HiX,1.0f);
	// Top Face
	meshAddVert(m, xa, yb, za, LoX,0.0f);
	meshAddVert(m, xa, yb, zb, LoX,1.0f);
	meshAddVert(m, xb, yb, zb, HiX,1.0f);
	meshAddVert(m, xb, yb, zb, HiX,1.0f);
	meshAddVert(m, xb, yb, za, HiX,0.0f);
	meshAddVert(m, xa, yb, za, LoX,0.0f);
	// Bottom Face
	meshAddVert(m, xa, ya, za, HiX,0.0f);
	meshAddVert(m, xb, ya, za, LoX,0.0f);
	meshAddVert(m, xb, ya, zb, LoX,1.0f);
	meshAddVert(m, xb, ya, zb, LoX,1.0f);
	meshAddVert(m, xa, ya, zb, HiX,1.0f);
	meshAddVert(m, xa, ya, za, HiX,0.0f);
	// Right face
	meshAddVert(m, xb, ya, za, HiX,1.0f);
	meshAddVert(m, xb, yb, za, HiX,0.0f);
	meshAddVert(m, xb, yb, zb, LoX,0.0f);
	meshAddVert(m, xb, yb, zb, LoX,0.0f);
	meshAddVert(m, xb, ya, zb, LoX,1.0f);
	meshAddVert(m, xb, ya, za, HiX,1.0f);
	// Left Face
	meshAddVert(m, xa, ya, za, LoX,1.0f);
	meshAddVert(m, xa, ya, zb, HiX,1.0f);
	meshAddVert(m, xa, yb, zb, HiX,0.0f);
	meshAddVert(m, xa, yb, zb, HiX,0.0f);
	meshAddVert(m, xa, yb, za, LoX,0.0f);
	meshAddVert(m, xa, ya, za, LoX,1.0f);

	return false;
}

void blockMiningDraw(){
	if(blockMiningCount == 0){return;}
	shaderBind(sMesh);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sMesh,matMVP);
	blockMiningProgressMesh->vertexCount = 0;
	for(int i=0;i<blockMiningCount;i++){
		blockMiningUpdateMesh(i);
		if(blockMiningList[i].lastDamage < blockMiningList[i].damage){
			fxBlockMine(vecNew(blockMiningList[i].x,blockMiningList[i].y,blockMiningList[i].z),blockMiningList[i].damage,blockMiningList[i].b);
		}
		blockMiningList[i].lastDamage = blockMiningList[i].damage;
	}
	meshFinishDynamic(blockMiningProgressMesh);

	glPolygonOffset(-8,-8);
	glEnable(GL_POLYGON_OFFSET_FILL);

	meshDraw(blockMiningProgressMesh);

	glPolygonOffset(0,0);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void blockMiningInit(){
	blockMiningProgressMesh = meshNew(NULL);
	blockMiningProgressMesh->tex = tBlockMining;
}

int blockMiningNew(int x,int y,int z){
	blockMining *bm;
	if(!inWorld(x,y,z)){return -1;}
	if(blockMiningCount >= (int)countof(blockMiningList)){return -1;}
	bm = &blockMiningList[blockMiningCount++];
	bm->x = x;
	bm->y = y;
	bm->z = z;
	bm->damage = 0;
	bm->wasMined = false;
	bm->b = worldGetB(x,y,z);
	return blockMiningCount-1;
}

void blockMiningMine(uint i, int dmg){
	blockMining *bm = &blockMiningList[i];

	bm->damage  += dmg;
	bm->wasMined = true;
}

int blockMiningMinePos(int dmg, int x, int y, int z){
	if(!inWorld(x,y,z)){return 1;}
	for(int i=0;i<blockMiningCount;i++){
		blockMining *bm = &blockMiningList[i];
		if(bm->x != x){continue;}
		if(bm->y != y){continue;}
		if(bm->z != z){continue;}
		blockMiningMine(i,dmg);
		return 0;
	}
	if(worldGetB(x,y,z) == 0){return 1;}
	int i = blockMiningNew(x,y,z);
	if(i >= 0){
		blockMiningMine(i,dmg);
	}
	return 0;
}

void blockMiningMineBlock(int x, int y, int z, u8 cause){
	(void)cause;
	const blockId b = worldGetB(x,y,z);
	if(b == 0){return;}
	if((b == I_Grass) || (b == I_Dry_Grass) || (b == I_Roots) || (b == I_Snow_Grass)){
		worldSetB(x,y,z,I_Dirt);
	}else{
		worldSetB(x,y,z,0);
	}
}

static void blockMiningDel(int i){
	blockMiningList[i] = blockMiningList[--blockMiningCount];
}

void blockMiningUpdateAll(){
	PROFILE_START();

	for(int i=blockMiningCount-1;i>=0;i--){
		blockMining *bm = &blockMiningList[i];
		if(!bm->wasMined){
			const int maxhp = blockTypeGetHealth(bm->b);
			bm->damage -= maxhp >> 5;
			if(bm->damage <= 0){blockMiningDel(i);}
		}else{
			bm->wasMined = false;
			if(bm->damage > blockTypeGetHealth(bm->b)){
				blockMiningMineBlock(bm->x,bm->y,bm->z,0);
				blockMiningDel(i);
			}
		}
	}

	PROFILE_STOP();
}

uint blockMiningGetActive(){
	return blockMiningCount;
}
