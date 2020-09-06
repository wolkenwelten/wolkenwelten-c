#include "../game/entity.h"

#include "../main.h"
#include "../sdl/sfx.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>
#include "../gfx/gl.h"

entity entityList[1<<14];
int entityCount = 0;
entity *entityFirstFree = NULL;

#define ENTITY_FADEOUT (128.f)

entity *entityNew(float x, float y, float z , float yaw, float pitch, float roll){
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

	e->x          = x;
	e->y          = y;
	e->z          = z;
	e->yaw        = yaw;
	e->pitch      = pitch;
	e->roll       = roll;

	e->nextFree   = NULL;
	e->curChungus = NULL;
	e->eMesh      = NULL;

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

void entityDraw(entity *e){
	float matMVP[16];
	if(e        == NULL){return;}
	if(e->eMesh == NULL){return;}

	matMov(matMVP,matView);
	matMulTrans(matMVP,e->x,e->y+e->yoff,e->z);
	matMulScale(matMVP,0.25f,0.25f,0.25f);
	matMulRotYX(matMVP,e->yaw,e->pitch);
	matMul(matMVP,matMVP,matProjection);

	shaderMatrix(sMesh,matMVP);
	meshDraw(e->eMesh);
}

void entityDrawAll(){
	shaderBind(sMesh);
	for(int i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(entityDistance(&entityList[i],player) > (ENTITY_FADEOUT * ENTITY_FADEOUT)){ continue; }
		entityDraw(&entityList[i]);
	}
}

void entityUpdateAll(){
	for(int i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(!entityList[i].updated){
			entityUpdate(&entityList[i]);
		}
		entityList[i].updated = false;
	}
}
