#include "../game/animal.h"

#include "../game/character.h"
#include "../gfx/effects.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../tmp/objs.h"
#include "../../../common/src/network/messages.h"

#include <stdlib.h>
#include <math.h>

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

static void animalDraw(animal *e){
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
	e->screenPos = matMulVec(matMVP,vecNew(0,0.25f,0));

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
	animalCount   = p->v.u16[5];
	const uint i  = p->v.u16[4];
	if (i >= animalCount){ return; }
	animal *e     = &animalList[i];

	e->pos        = vecNewP(&p->v.f[3]);
	e->vel        = vecNewP(&p->v.f[6]);
	e->gvel       = vecNewP(&p->v.f[9]);
	e->rot.yaw    = p->v.f[12];
	e->rot.pitch  = p->v.f[13];

	e->grot.yaw   = p->v.f[14];
	e->grot.pitch = p->v.f[15];

	e->type       = p->v.u8[ 0];
	e->flags      = p->v.u8[ 1];
	e->state      = p->v.u8[ 2];

	e->age        = p->v.i8[ 3];
	e->health     = p->v.i8[ 4];
	e->hunger     = p->v.i8[ 5];
	e->thirst     = p->v.i8[ 6];
	e->sleepy     = p->v.i8[ 7];
}

int animalHitCheck(const vec pos, float mdd, int dmg, int cause, uint iteration){
	int hits = 0;
	for(uint i=0;i<animalCount;i++){
		if(animalList[i].temp == iteration){continue;}
		const vec d = vecSub(pos,animalList[i].pos);
		if(vecDot(d,d) < mdd){
			msgBeingDamage(0,dmg,cause,beingAnimal(i),0,pos);
			animalList[i].temp = iteration;
			hits++;
		}
	}
	return hits;
}

void animalGotHitPacket(const packet *p){
	const being target  = p->v.u32[1];
	if(beingType(target) != BEING_ANIMAL){return;}
	if(beingID(target) > animalCount)    {return;}
	const animal *c = &animalList[beingID(target)];
	fxBleeding(c->pos,target,p->v.i16[0],p->v.u16[1]);
}
