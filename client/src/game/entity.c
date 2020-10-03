#include "../game/entity.h"

#include "../main.h"
#include "../sdl/sfx.h"
#include "../game/character.h"
#include "../gfx/frustum.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/shadow.h"
#include "../gfx/texture.h"
#include "../tmp/assets.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>
#include "../gfx/gl.h"

entity  entityList[1<<14];
int     entityCount = 0;
entity *entityFirstFree = NULL;

#define ENTITY_FADEOUT (128.f)

entity *entityNew(vec pos, vec rot){
	entity *e = NULL;
	if(entityFirstFree == NULL){
		e = &entityList[entityCount++];
	}else{
		e = entityFirstFree;
		entityFirstFree = e->nextFree;
		if(entityFirstFree == e){
			entityFirstFree = NULL;
		}
	}
	entityReset(e);

	e->pos = pos;
	e->rot = rot;

	return e;
}

void entityFree(entity *e){
	if(e == NULL){return;}
	e->nextFree = entityFirstFree;
	entityFirstFree = e;
	if(e->nextFree == NULL){
		e->nextFree = e;
	}
}

void entityDraw(const entity *e){
	if(e->eMesh == NULL){return;}

	matMov      (matMVP,matView);
	matMulTrans (matMVP,e->pos.x,e->pos.y+e->yoff,e->pos.z);
	matMulScale (matMVP,0.4f,0.4f,0.4f);
	matMulRotYX (matMVP,e->rot.yaw,e->rot.pitch);
	matMul      (matMVP,matMVP,matProjection);

	shaderMatrix(sShadow,matMVP);
	meshDraw(e->eMesh);
	shadowAdd(e->pos,0.5f);
}

void entityDrawAll(){
	shaderBind(sMesh);
	for(int i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(entityDistance(&entityList[i],player) > (ENTITY_FADEOUT * ENTITY_FADEOUT)){ continue; }
		if(!CubeInFrustum(vecSubS(entityList[i].pos,.5f),1.f)){continue;}
		entityDraw(&entityList[i]);
	}
}

void entityUpdateAll(){
	for(int i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(!(entityList[i].flags & ENTITY_UPDATED)){
			entityUpdate(&entityList[i]);
		}
		entityList[i].flags &= ~ENTITY_UPDATED;
	}
}
