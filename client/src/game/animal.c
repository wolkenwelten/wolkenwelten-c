#include "../game/animal.h"

#include "../game/character.h"
#include "../gfx/effects.h"
#include "../gfx/gfx.h"
#include "../gfx/particle.h"
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

#define ANIMAL_FADEOUT (256.f)

static const mesh *animalGetMesh(const animal *e){
	switch(e->type){
	default:
		return meshKarnickel;
	case 2:
		return meshBeamguardian;
	}
}

static void animalDraw(animal *e){
	float breath,scale;
	if(e        == NULL){return;}
	if(e->type  == 0)   {return;}
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
	e->screenPos = matMulVec(matMVP,vecNew(0,scale/2.f,0));

	shaderMatrix(sMesh,matMVP);
	meshDraw(animalGetMesh(e));
}

void animalDrawAll(){
	shaderBind(sMesh);
	for(uint i=0;i<animalCount;i++){
		if(animalDistance(&animalList[i],player) > ANIMAL_FADEOUT){
			animalList[i].screenPos = vecNOne();
			continue;
		}
		animalDraw(&animalList[i]);
	}
}

static void animalUpdateClientside(animal *e){
	if((e->type == 2) && (e->state == ANIMAL_S_FIGHT)){
		for(int i=0;i<4;i++){
			const vec v = vecMulS(vecRng(),(1.f/128.f));
			newParticleV(e->pos,v,vecMulS(v,1/64.f),32.f,2.f,0xFF964AC0,192);
		}
		for(int i=0;i<4;i++){
			const vec v = vecMulS(vecRng(),(1.f/156.f));
			newParticleV(e->pos,v,vecMulS(v,1/-96.f),16.f,4.f,0xFF7730A0,154);
		}
	}
}

void animalUpdateAll(){
	for(uint i=0;i<animalCount;i++){
		animalUpdate(&animalList[i]);
		animalUpdateClientside(&animalList[i]);
	}
}

void animalSyncFromServer(const packet *p){
	const uint newC = p->v.u16[5];
	if(newC > animalCount){
		for(uint ii=animalCount;ii<newC;ii++){
			animalList[ii].type = 0;
		}
	}
	animalCount   = newC;
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
	e->pregnancy  = p->v.i8[ 6];
	e->sleepy     = p->v.i8[ 7];
}

int animalHitCheck(const vec pos, float mdd, int dmg, int cause, u16 iteration){
	int hits = 0;
	for(uint i=0;i<animalCount;i++){
		if(animalList[i].type == 0)        {continue;}
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
