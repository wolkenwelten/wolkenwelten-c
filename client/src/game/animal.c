#include "../game/animal.h"

#include "../main.h"
#include "../sdl/sfx.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/particle.h"
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
	e->yoff       = 0.f;
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

void aniomalShadesDraw(animal *c){
	float matMVP[16],breath,scale=0.5f;
	if(c->state == 0){
		breath = sinf((float)(c->breathing-256)/512.f)*2.f;
	}else{
		breath = sinf((float)(c->breathing-128)/256.f)*2.f;
	}
	if(c->flags & ANIMAL_YOUNG){
		scale = 0.25f;
	}
	
	matMov(matMVP,matView);
	matMulTrans(matMVP,c->x,c->y+c->yoff+breath/128.f,c->z);
	matMulRotYX(matMVP,c->yaw,c->pitch + breath);
	matMulTrans(matMVP,0.f,0.1f,-0.2f);
	matMulScale(matMVP,scale,scale,scale);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(meshSunglasses);
}

void animalDraw(animal *e){
	float matMVP[16],breath,scale = 1.f;
	if(e        == NULL){return;}
	if(e->state == 0){
		breath = cosf((float)e->breathing/512.f)*4.f;
	}else{
		breath = cosf((float)e->breathing/256.f)*6.f;
	}
	if(e->flags & ANIMAL_YOUNG){
		scale = 0.5f;
	}

	matMov      (matMVP,matView);
	matMulTrans (matMVP,e->x,e->y+e->yoff,e->z);
	matMulRotYX (matMVP,e->yaw,e->pitch-breath);
	matMulScale (matMVP,scale,scale,scale);
	matMul      (matMVP,matMVP,matProjection);

	shaderMatrix(sMesh,matMVP);
	meshDraw(meshPear);
	aniomalShadesDraw(e);
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
	animalCount = p->val.u[ 2];
	int i       = p->val.u[ 1];
	animal *e   = &animalList[i];
	if(i >= animalCount){return;}

	e->x     = p->val.f[ 3];
	e->y     = p->val.f[ 4];
	e->z     = p->val.f[ 5];
	e->yaw   = p->val.f[ 6];
	e->pitch = p->val.f[ 7];
	e->roll  = p->val.f[ 8];
	e->vx    = p->val.f[ 9];
	e->vy    = p->val.f[10];
	e->vz    = p->val.f[11];
	e->yoff  = p->val.f[12];
	
	e->type  = p->val.c[ 0];
	e->flags = p->val.c[ 1];
	e->state = p->val.c[ 2];
}
