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
uint    animalCount = 0;
animal *animalFirstFree = NULL;

#define ANIMAL_FADEOUT (128.f)

animal *animalNew(float x, float y, float z , int type){
	animal *e = NULL;
	if(animalCount >= ((sizeof(animalList) / sizeof(animal))-1)){return NULL;}
	e = &animalList[animalCount++];
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

	e->curChungus = NULL;

	return e;
}

void aniomalShadesDraw(animal *c){
	float matMVP[16],breath,scale=0.5f;
	if(c->state == 0){
		breath = sinf((float)(c->breathing-256)/512.f)*2.f;
	}else{
		breath = sinf((float)(c->breathing-128)/256.f)*2.f;
	}
	
	if(c->age < 20){
		scale = 0.5f + ((float)c->age/40.f);
	}else{
		scale = 1.f;
	}
	scale *= 0.5f;
	
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
	if(e->age < 20){
		scale = 0.5f + ((float)e->age/40.f);
	}else{
		scale = 1.f;
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
	for(uint i=0;i<animalCount;i++){
		if(animalDistance(&animalList[i],player) > (ANIMAL_FADEOUT * ANIMAL_FADEOUT)){ continue; }
		animalDraw(&animalList[i]);
	}
}

void animalUpdateAll(){
	for(uint i=0;i<animalCount;i++){
		animalUpdate(&animalList[i]);
	}
}

void animalSyncFromServer(packet *p){
	animalCount = p->val.u[ 3];
	uint i      = p->val.u[ 2];
	if (i >= animalCount){ return; }
	animal *e   = &animalList[i];

	e->x        = p->val.f[ 4];
	e->y        = p->val.f[ 5];
	e->z        = p->val.f[ 6];
	e->yaw      = p->val.f[ 7];
	e->pitch    = p->val.f[ 8];
	e->roll     = p->val.f[ 9];
	e->vx       = p->val.f[10];
	e->vy       = p->val.f[11];
	e->vz       = p->val.f[12];
	e->yoff     = p->val.f[13];
	
	e->type     = p->val.c[ 0];
	e->flags    = p->val.c[ 1];
	e->state    = p->val.c[ 2];
	
	e->age      = p->val.c[ 3];
	e->health   = p->val.c[ 4];
	e->hunger   = p->val.c[ 5];
	e->thirst   = p->val.c[ 6];
	e->sleepy   = p->val.c[ 7];
}
