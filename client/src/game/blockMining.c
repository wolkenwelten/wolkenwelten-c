#include "../game/blockMining.h"

#include "../game/blockType.h"
#include "../game/itemDrop.h"
#include "../gfx/effects.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../../../common/src/misc.h"
#include "../../../common/src/messages.h"
#include "../tmp/assets.h"
#include "../voxel/bigchungus.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include "../gfx/glew.h"

typedef struct {
	int x,y,z;
	int damage;
	int lastDamage;
	uint8_t b;
	int wasMined;
} blockMining;

blockMining blockMiningList[128];
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
	const float mw = 1.08f;
	const float mh = 1.08f;
	const float md = 1.08f;
	const float mx = bm->x-0.04f;
	const float my = bm->y-0.04f;
	const float mz = bm->z-0.04f;
	mesh *m = blockMiningProgressMesh;

	// Front Face
	meshAddVert(m, mx   ,my   ,mz+md,LoX,1.0f);
	meshAddVert(m, mx+mw,my   ,mz+md,HiX,1.0f);
	meshAddVert(m, mx+mw,my+mh,mz+md,HiX,0.0f);
	meshAddVert(m, mx+mw,my+mh,mz+md,HiX,0.0f);
	meshAddVert(m, mx   ,my+mh,mz+md,LoX,0.0f);
	meshAddVert(m, mx   ,my   ,mz+md,LoX,1.0f);
	// Back Face
	meshAddVert(m, mx   ,my   ,mz   ,HiX,1.0f);
	meshAddVert(m, mx   ,my+mh,mz   ,HiX,0.0f);
	meshAddVert(m, mx+mw,my+mh,mz   ,LoX,0.0f);
	meshAddVert(m, mx+mw,my+mh,mz   ,LoX,0.0f);
	meshAddVert(m, mx+mw,my   ,mz   ,LoX,1.0f);
	meshAddVert(m, mx   ,my   ,mz   ,HiX,1.0f);
	// Top Face
	meshAddVert(m, mx   ,my+mh,mz   ,LoX,0.0f);
	meshAddVert(m, mx   ,my+mh,mz+md,LoX,1.0f);
	meshAddVert(m, mx+mw,my+mh,mz+md,HiX,1.0f);
	meshAddVert(m, mx+mw,my+mh,mz+md,HiX,1.0f);
	meshAddVert(m, mx+mw,my+mh,mz   ,HiX,0.0f);
	meshAddVert(m, mx   ,my+mh,mz   ,LoX,0.0f);
	// Bottom Face
	meshAddVert(m, mx   ,my   ,mz   ,HiX,0.0f);
	meshAddVert(m, mx+mw,my   ,mz   ,LoX,0.0f);
	meshAddVert(m, mx+mw,my   ,mz+md,LoX,1.0f);
	meshAddVert(m, mx+mw,my   ,mz+md,LoX,1.0f);
	meshAddVert(m, mx   ,my   ,mz+md,HiX,1.0f);
	meshAddVert(m, mx   ,my   ,mz   ,HiX,0.0f);
	// Right face
	meshAddVert(m, mx+mw,my   ,mz   ,HiX,1.0f);
	meshAddVert(m, mx+mw,my+mh,mz   ,HiX,0.0f);
	meshAddVert(m, mx+mw,my+mh,mz+md,LoX,0.0f);
	meshAddVert(m, mx+mw,my+mh,mz+md,LoX,0.0f);
	meshAddVert(m, mx+mw,my   ,mz+md,LoX,1.0f);
	meshAddVert(m, mx+mw,my   ,mz   ,HiX,1.0f);
	// Left Face
	meshAddVert(m, mx   ,my   ,mz   ,LoX,1.0f);
	meshAddVert(m, mx   ,my   ,mz+md,HiX,1.0f);
	meshAddVert(m, mx   ,my+mh,mz+md,HiX,0.0f);
	meshAddVert(m, mx   ,my+mh,mz+md,HiX,0.0f);
	meshAddVert(m, mx   ,my+mh,mz   ,LoX,0.0f);
	meshAddVert(m, mx   ,my   ,mz   ,LoX,1.0f);

	return false;
}

void blockMiningDraw(){
	float matMVP[16];
	if(blockMiningCount == 0){return;}
	shaderBind(sMesh);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sMesh,matMVP);
	blockMiningProgressMesh->dataCount = 0;
	for(int i=0;i<blockMiningCount;i++){
		if(blockMiningUpdateMesh(i)){
			i--;
			continue;
		}
		if(blockMiningList[i].lastDamage < blockMiningList[i].damage){
			fxBlockMine(blockMiningList[i].x,blockMiningList[i].y,blockMiningList[i].z,blockMiningList[i].damage,blockMiningList[i].b);
		}
		blockMiningList[i].lastDamage = blockMiningList[i].damage;
	}
	meshFinish(blockMiningProgressMesh, GL_STREAM_DRAW);
	meshDraw(blockMiningProgressMesh);
}

void blockMiningInit(){
	blockMiningProgressMesh = meshNew();
	blockMiningProgressMesh->tex = tBlockMining;
}

void blockMiningUpdateFromServer(packet *p){
	if(p->val.i[2] < blockMiningCount){
		blockMiningCount = p->val.i[2];
	}
	if((p->val.i[3] >= blockMiningCount) && (p->val.i[3] < p->val.i[2])){
		blockMiningCount = p->val.i[3]+1;
	}
	blockMining *bm = &blockMiningList[p->val.i[3]];
	bm->x      =  p->val.i[0]        & 0xFFFF;
	bm->y      = (p->val.i[0] >> 16) & 0xFFFF;
	bm->z      =  p->val.i[1]        & 0xFFFF;
	bm->damage = (p->val.i[1] >> 16) & 0xFFFF;
	bm->b      = worldGetB(bm->x,bm->y,bm->z);
}

