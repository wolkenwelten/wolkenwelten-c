#include "../game/animal.h"

#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../tmp/objs.h"

#include <stdlib.h>
#include <math.h>
#include "../gfx/gl.h"

animal  animalList[1<<10];
uint    animalCount = 0;
animal *animalFirstFree = NULL;

#define ANIMAL_FADEOUT (128.f)

static void animalShadesDraw(const animal *c){
	float breath,scale;
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
	matMulTrans(matMVP,c->pos.x,c->pos.y+breath/128.f,c->pos.z);
	matMulRotYX(matMVP,c->pos.yaw,c->pos.pitch + breath);
	matMulTrans(matMVP,0.f,0.1f,-0.2f);
	matMulScale(matMVP,scale,scale,scale);
	matMul(matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw(meshSunglasses);
}

static void animalDraw(const animal *e){
	float breath,scale;
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
	matMulTrans (matMVP,e->pos.x,e->pos.y,e->pos.z);
	matMulRotYX (matMVP,e->rot.yaw,e->rot.pitch-breath);
	matMulScale (matMVP,scale,scale,scale);
	matMul      (matMVP,matMVP,matProjection);

	shaderMatrix(sMesh,matMVP);
	meshDraw(meshPear);
	animalShadesDraw(e);
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

void animalSyncFromServer(const packet *p){
	animalCount   = p->val.u[ 3];
	uint i        = p->val.u[ 2];
	if (i >= animalCount){ return; }
	animal *e     = &animalList[i];

	e->pos.x      = p->val.f[ 4];
	e->pos.y      = p->val.f[ 5];
	e->pos.z      = p->val.f[ 6];
	e->vel.x      = p->val.f[ 7];
	e->vel.y      = p->val.f[ 8];
	e->vel.z      = p->val.f[ 9];
	e->gvel.x     = p->val.f[10];
	e->gvel.y     = p->val.f[11];
	e->gvel.z     = p->val.f[12];
	e->rot.yaw    = p->val.f[13];
	e->rot.pitch  = p->val.f[14];
	e->grot.yaw   = p->val.f[15];
	e->grot.pitch = p->val.f[16];

	e->type       = p->val.c[ 0];
	e->flags      = p->val.c[ 1];
	e->state      = p->val.c[ 2];

	e->age        = p->val.c[ 3];
	e->health     = p->val.c[ 4];
	e->hunger     = p->val.c[ 5];
	e->thirst     = p->val.c[ 6];
	e->sleepy     = p->val.c[ 7];
}
