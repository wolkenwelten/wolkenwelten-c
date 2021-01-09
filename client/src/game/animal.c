#include "../game/animal.h"

#include "../game/being.h"
#include "../game/character.h"
#include "../gfx/effects.h"
#include "../gfx/gfx.h"
#include "../gfx/particle.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/shadow.h"
#include "../gfx/sky.h"
#include "../tmp/objs.h"
#include "../../../common/src/game/time.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ANIMAL_FADEOUT (256.f)

static const mesh *animalGetMesh(const animal *e){
	switch(e->type){
	default:
		return meshBunny;
	case 1:
		return meshBunny;
	case 2:
		return meshBeamguardian;
	case 3:
		return meshWerebunny;
	}
}

static void animalDraw(animal *e){
	float breath;
	if(e        == NULL){return;}
	if(e->type  == 0)   {return;}
	if(e->state == 0){
		breath = cosf((float)e->breathing/512.f)*4.f;
	}else{
		breath = cosf((float)e->breathing/256.f)*6.f;
	}
	float scale = MIN(1.f,0.5f + ((float)e->age/40.f));
	if(e->type == 3){
		const float sunlight = gtimeGetBrightness(gtimeGetTimeOfDay());
		scale *= (MAX(0.f,(((1.f - sunlight) - 0.5f) * 5.f))+1.f);
	}

	matMov      (matMVP,matView);
	matMulTrans (matMVP,e->pos.x,e->pos.y,e->pos.z);
	matMulRotYX (matMVP,e->rot.yaw,e->rot.pitch-breath);
	matMulScale (matMVP,scale,scale,scale);
	matMul      (matMVP,matMVP,matProjection);
	e->screenPos = matMulVec(matMVP,vecNew(0,scale/2.f,0));

	shaderMatrix(sMesh,matMVP);
	meshDraw(animalGetMesh(e));
	shadowAdd(e->pos,scale*1.5f);
}

void animalDrawAll(){
	shaderBind(sMesh);
	shaderBrightness(sMesh,skyBrightness);
	for(uint i=0;i<animalCount;i++){
		if(animalDistance(&animalList[i],player) > ANIMAL_FADEOUT){
			animalList[i].screenPos = vecNOne();
			continue;
		}
		animalDraw(&animalList[i]);
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

	e->target     = p->v.u32[16];

	e->type       = p->v.u8[ 0];
	e->flags      = p->v.u8[ 1];
	e->state      = p->v.u8[ 2];

	e->age        = p->v.i8[ 3];
	e->health     = p->v.i8[ 4];
	e->hunger     = p->v.i8[ 5];
	e->pregnancy  = p->v.i8[ 6];
	e->sleepy     = p->v.i8[ 7];

	if(e->type == 0){
		beingListDel(e->bl,animalGetBeing(e));
	}else{
		e->bl = beingListUpdate(e->bl,animalGetBeing(e));
	}
}

void animalGotHitPacket(const packet *p){
	const being target  = p->v.u32[1];
	if(beingType(target) != BEING_ANIMAL){return;}
	if(beingID(target) > animalCount)    {return;}
	const animal *c = &animalList[beingID(target)];
	fxBleeding(c->pos,target,p->v.i16[0],p->v.u16[1]);
}
