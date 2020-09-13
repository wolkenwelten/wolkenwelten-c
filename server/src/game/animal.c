#include "animal.h"

#include "../main.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

animal  animalList[1<<10];
int     animalCount = 0;
animal *animalFirstFree = NULL;

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
	e->breathing  = 0;

	e->health     = 20;
	e->hunger     = 64;
	e->thirst     = 64;
	e->sleepy     = 64;

	e->flags      = 0;
	e->type       = type;
	e->state      = 0;

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

void animalDel(int i){
	if(i >= animalCount) {return;}
	if(i < 0)            {return;}
	animalList[i] = animalList[--animalCount];
}

void animalUpdateAll(){
	for(int i=animalCount-1;i>=0;i--){
		if(animalList[i].nextFree != NULL){ continue; }
		int dmg = animalUpdate(&animalList[i]);
		animalList[i].health -= dmg;
		if((animalList[i].y < -256.f) ||
		   (animalList[i].health < 0) ||
		   (animalList[i].hunger < 0) ||
		   (animalList[i].thirst < 0) ||
		   (animalList[i].sleepy < 0)) {
			animalDel(i);
			continue;
		}
	}
}

float animalClosestPlayer(animal *e, character **cChar){
	*cChar = NULL;
	float ret = 4096.f;
	for(int i=0;i<clientCount;++i){
		if(clients[i].c == NULL){continue;}
		float dx = clients[i].c->x - e->x;
		float dy = clients[i].c->y - e->y;
		float dz = clients[i].c->z - e->z;
		float d  = (dx*dx)+(dy*dy)+(dz*dz);
		if(d < ret){
			ret = d;
			*cChar = clients[i].c;
		}
	}
	return ret;
}

float animalClosestAnimal(animal *e, animal **cAnim, int typeFilter){
	*cAnim = NULL;
	float ret = 4096.f;
	for(int i=0;i<animalCount;i++){
		if(e == &animalList[i])                                    {continue;}
		if(animalList[i].nextFree != NULL)                         {continue;}
		if((typeFilter >= 0) && (animalList[i].type != typeFilter)){continue;}
		float dx = animalList[i].x - e->x;
		float dy = animalList[i].y - e->y;
		float dz = animalList[i].z - e->z;
		float d  = (dx*dx)+(dy*dy)+(dz*dz);
		if(d < ret){
			ret = d;
			*cAnim = &animalList[i];
		}
	}
	return ret;
}

void animalSLoiter(animal *e){
	character *cChar;
	animal *cAnim;
	float dist = animalClosestPlayer(e,&cChar);
	if((dist < 24.f) && (cChar != NULL)){
		e->vy += 0.03;
		e->state = 1;
		return;
	}

	if(e->flags & ANIMAL_STUFFED){
		if(rngValM(1<<10)){
			e->flags &= ~ANIMAL_STUFFED;
		}
	}else{
		const uint8_t cb = worldGetB(e->x,e->y-.6f,e->z);
		if((cb == 2) && (rngValM(128) == 0)){
			worldSetB(e->x,e->y-.6f,e->z,1);
			e->flags |= ANIMAL_STUFFED;
		}
	}

	if(e->flags & ANIMAL_YOUNG){
		if(rngValM(1<<13) == 0){ e->flags &= ~ANIMAL_YOUNG; }
	}else if(e->flags & ANIMAL_STUFFED){
		if(e->flags & ANIMAL_EXHAUSTED){
			if(rngValM(1<<10) == 0){ e->flags &= ~ANIMAL_EXHAUSTED; }
		}else if(rngValM( 128) == 0){
			if(animalClosestAnimal(e,&cAnim,e->type) < 192.f){
				if(!(cAnim->flags & ANIMAL_YOUNG) && !(cAnim->flags & ANIMAL_EXHAUSTED)){
					e->state = 2;
					return;
				}
			}
		}
	}

	if(rngValM( 128) == 0){
		e->gyaw = e->yaw + ((rngValf()*2.f)-1.f)*4.f;
	}
	if(rngValM(  64) == 0){
		e->gvx = 0;
		e->gvy = 0;
		e->gvz = 0;
	}
	if(rngValM(1024) == 0){
		e->gpitch = ((rngValf()*2.f)-1.f)*10.f;
	}
	if(rngValM(2048) == 0){
		e->gyaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValM( 512) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->yaw,0.f,0.f)),0.01f);
		e->gvx = dir.x;
		e->gvz = dir.z;
	}
}

void animalSHorny(animal *e){
	animal *cAnim;
	float dist = animalClosestAnimal(e,&cAnim,e->type);
	if((dist > 256.f) || (cAnim == NULL)){ e->state = 0; }
	if(dist < 2.f){
		e->state = 0;
		cAnim = animalNew(e->x+((rngValf()*2.f)-1.f),e->y,e->z+((rngValf()*2.f)-1.f),e->type);
		cAnim->flags |= ANIMAL_YOUNG;
		e->flags |= ANIMAL_EXHAUSTED;
	}

	vec caNorm = vecNorm(vecNew(e->x - cAnim->x,e->y - cAnim->y, e->z - cAnim->z));
	vec caVel  = vecMulS(caNorm,-0.03f);
	vec caRot  = vecVecToDeg(caNorm);

	e->gvx = caVel.x;
	e->gvz = caVel.z;

	e->yaw = -caRot.yaw + 180.f;
}

void animalSFlee(animal *e){
	character *cChar;
	const float dist = animalClosestPlayer(e,&cChar);
	if((dist > 96.f) && (cChar != NULL)){
		e->state = 0;
		return;
	}
	if(cChar != NULL){
		vec caNorm = vecNorm(vecNew(e->x - cChar->x,e->y - cChar->y, e->z - cChar->z));
		vec caVel  = vecMulS(caNorm,0.03f);
		vec caRot  = vecVecToDeg(caNorm);

		e->gvx = caVel.x;
		e->gvz = caVel.z;

		e->yaw = -caRot.yaw;
	}
	if(e->flags & ANIMAL_YOUNG){
		if(rngValM(1<<14) == 0){ e->flags &= ~ANIMAL_YOUNG; }
	}
}

inline static void animalThink(animal *e){
	if(e == NULL){return;}
	switch(e->state){
		default:
		case 0:
			animalSLoiter(e);
			break;
		case 1:
			animalSFlee(e);
			break;
		case 2:
			animalSHorny(e);
			break;
	}

}

void animalThinkAll(){
	for(int i=animalCount-1;i>=0;--i){
		if(animalList[i].nextFree != NULL){ continue; }
		animalThink(&animalList[i]);
	}
}

void animalEmptySync(int c){
	packet *rp = &packetBuffer;

	rp->val.u[ 0] = 0;
	rp->val.u[ 1] = 0;
	rp->val.u[ 2] = 0;

	packetQueue(rp,30,13*4,c);
}

void animalSync(int c, int i){
	packet *rp = &packetBuffer;
	animal *e = &animalList[i];

	rp->val.c[ 0] = e->type;
	rp->val.c[ 1] = e->flags;
	rp->val.c[ 2] = e->state;
	rp->val.c[ 3] = 0;

	rp->val.u[ 1] = i;
	rp->val.u[ 2] = animalCount;

	rp->val.f[ 3] = e->x;
	rp->val.f[ 4] = e->y;
	rp->val.f[ 5] = e->z;
	rp->val.f[ 6] = e->yaw;
	rp->val.f[ 7] = e->pitch;
	rp->val.f[ 8] = e->roll;
	rp->val.f[ 9] = e->vx;
	rp->val.f[10] = e->vy;
	rp->val.f[11] = e->vz;
	rp->val.f[12] = e->yoff;

	packetQueue(rp,30,13*4,c);
}

void animalSyncPlayer(int c){
	if(animalCount == 0){
		animalEmptySync(c);
	}
	for(int i=0;i<animalCount;i++){
		animalSync(c,i);
	}
}

uint8_t *animalSave(animal *e, uint8_t *b){
	float *f = (float *)b;
	if(e == NULL){return b;}

	b[ 0] = 0x03;
	b[ 1] = e->flags;
	b[ 2] = e->type;
	b[ 3] = e->state;

	f[ 1] = e->x;
	f[ 2] = e->y;
	f[ 3] = e->z;
	f[ 4] = e->yaw;
	f[ 5] = e->pitch;
	f[ 6] = e->roll;
	f[ 7] = e->vx;
	f[ 8] = e->vy;
	f[ 9] = e->vz;
	f[10] = e->yoff;

	return b+11*4;
}

uint8_t *animalLoad(uint8_t *b){
	float *f = (float *)b;
	animal *e   = animalNew(f[1],f[2],f[3],b[2]);
	if(e == NULL){return b+11*4;}

	e->yaw   = f[ 4];
	e->pitch = f[ 5];
	e->roll  = f[ 6];
	e->vx    = f[ 7];
	e->vy    = f[ 8];
	e->vz    = f[ 9];
	e->yoff  = f[10];

	e->flags = b[ 1];
	e->state = b[ 3];

	return b+11*4;
}

uint8_t *animalSaveChungus(chungus *c,uint8_t *b){
	if(c == NULL){return b;}
	for(int i=0;i<animalCount;i++){
		if(animalList[i].curChungus != c){continue;}
		b = animalSave(&animalList[i],b);
	}
	return b;
}

void animalDelChungus(chungus *c){
	if(c == NULL){return;}
	for(int i=animalCount-1;i>=0;i--){
		if(animalList[i].curChungus != c){continue;}
		animalDel(i);
	}
}
