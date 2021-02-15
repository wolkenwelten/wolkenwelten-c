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
#include "../game/itemDrop.h"
#include "../gfx/effects.h"
#include "../gfx/gl.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../tmp/assets.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdlib.h>

typedef struct {
	u16 x,y,z;
	int damage;
	int lastDamage;
	u8  b;
} blockMining;

blockMining blockMiningList[4096];
int         blockMiningCount = 0;
mesh       *blockMiningProgressMesh;

float blockMiningGetProgress(blockMining *bm){
	if(bm->b == 0){return 0.f;}
	return ((float)bm->damage) / ((float)blockTypeGetHP(bm->b));
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
	blockMiningProgressMesh->dataCount = 0;
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

void blockMiningUpdateFromServer(const packet *p){
	blockMiningCount = p->v.u16[5];
	const int i = p->v.u16[4];
	if(i >= blockMiningCount){return;}
	if(i >= (int)countof(blockMiningList)){return;}
	blockMining *bm = &blockMiningList[i];
	bm->x      = p->v.u16[0];
	bm->y      = p->v.u16[1];
	bm->z      = p->v.u16[2];
	bm->damage = p->v.i16[3];
	bm->b      = worldGetB(bm->x,bm->y,bm->z);
}
