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
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdlib.h>

typedef struct {
	int x,y,z;
	int damage;
	int lastDamage;
	u8  b;
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
			fxBlockMine(vecNew(blockMiningList[i].x,blockMiningList[i].y,blockMiningList[i].z),blockMiningList[i].damage,blockMiningList[i].b);
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

void blockMiningUpdateFromServer(const packet *p){
	blockMiningCount = p->v.u16[5];
	const int i = p->v.u16[4];
	if(i >= blockMiningCount){return;}
	blockMining *bm = &blockMiningList[i];
	bm->x      = p->v.u16[0];
	bm->y      = p->v.u16[1];
	bm->z      = p->v.u16[2];
	bm->damage = p->v.i16[3];
	bm->b      = worldGetB(bm->x,bm->y,bm->z);
}
