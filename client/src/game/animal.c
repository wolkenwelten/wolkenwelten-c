#include "../game/animal.h"

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

animal  animalList[1<<10];
int     animalCount = 0;
animal *animalFirstFree = NULL;

#define ANIMAL_FADEOUT (128.f)

animal *animalNew(float x, float y, float z , int type){
	animal *e = NULL;
	if(animalFirstFree == NULL){
		e = &animalList[animalCount++];
	}else{
		e = animalFirstFree;
		animalFirstFree = e->nextFree;
		if(animalFirstFree == e){
			animalFirstFree = NULL;
		}
	}
	animalReset(e);

	e->x          = x;
	e->y          = y;
	e->z          = z;
	e->yaw        = 0.f;
	e->pitch      = 0.f;
	e->roll       = 0.f;
	e->flags      = 0;
	e->type       = type;

	e->nextFree   = NULL;
	e->curChungus = NULL;

	return e;
}

void animalFree(animal *e){
	if(e == NULL){return;}
	e->nextFree = animalFirstFree;
	animalFirstFree = e;
	if(e->nextFree == NULL){
		e->nextFree = e;
	}
}

void animalDraw(animal *e){
	float matMVP[16];
	if(e        == NULL){return;}

	matMov      (matMVP,matView);
	matMulTrans (matMVP,e->x,e->y+e->yoff,e->z);
	matMulRotYX (matMVP,e->yaw,e->pitch);
	matMul      (matMVP,matMVP,matProjection);

	shaderMatrix(sMesh,matMVP);
	meshDraw(meshPear);
}

void animalDrawAll(){
	shaderBind(sMesh);
	for(int i=0;i<animalCount;i++){
		if(animalList[i].nextFree != NULL){ continue; }
		if(animalDistance(&animalList[i],player) > (ANIMAL_FADEOUT * ANIMAL_FADEOUT)){ continue; }
		animalDraw(&animalList[i]);
	}
}

void animalUpdateAll(){
	for(int i=0;i<animalCount;i++){
		if(animalList[i].nextFree != NULL){ continue; }
		animalUpdate(&animalList[i]);
	}
}

void animalSyncFromServer(packet *p){
	animalCount = p->val.u[13];
	int i       = p->val.u[12];
	animal *e   = &animalList[i];
	if(i >= animalCount){return;}

	e->x     = p->val.f[ 0];
	e->y     = p->val.f[ 1];
	e->z     = p->val.f[ 2];
	e->yaw   = p->val.f[ 3];
	e->pitch = p->val.f[ 4];
	e->roll  = p->val.f[ 5];
	e->vx    = p->val.f[ 6];
	e->vy    = p->val.f[ 7];
	e->vz    = p->val.f[ 8];
	e->yoff  = p->val.f[ 9];
	e->flags = p->val.u[10];
	e->type  = p->val.i[11];
}
