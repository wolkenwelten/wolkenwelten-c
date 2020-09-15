#include "animal.h"

#include "../main.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

animal       animalList[1<<10];
unsigned int animalCount = 0;
animal      *animalFirstFree = NULL;

#define ANIMALS_PER_UPDATE 16u

animal *animalNew(float x, float y, float z , int type){
	animal *e = NULL;
	if(animalCount >= ((sizeof(animalList) / sizeof(animal)-1))){return NULL;}
	e = &animalList[animalCount++];
	animalReset(e);

	e->x         = x;
	e->y         = y;
	e->z         = z;
	e->yaw       = 0.f;
	e->pitch     = 0.f;
	e->roll      = 0.f;
	e->breathing = 0;

	e->age       = 21;
	e->health    = 20;
	e->hunger    = 64;
	e->thirst    = 64;
	e->sleepy    = 64;

	e->flags     = 0;
	e->type      = type;
	e->state     = 0;

	if(rngValM(2) == 0){
		e->flags |= ANIMAL_BELLYSLEEP;
	}

	e->curChungus = NULL;

	return e;
}

static void animalDel(uint i){
	if(i >= animalCount) {return;}
	animalList[i] = animalList[--animalCount];
}

void animalUpdateAll(){
	for(int i=animalCount-1;i>=0;i--){
		int dmg = animalUpdate(&animalList[i]);
		animalList[i].health -= dmg;
		if((animalList[i].y < -256.f) ||
		   (animalList[i].health < 0) ||
		   (animalList[i].hunger < 0) ||
		   (animalList[i].thirst < 0) ||
		   (animalList[i].sleepy < 0)) {
			fprintf(stderr,"Dead Animal [HP: %i | HUN: %i | THI: %i | SLP: %i]\n",animalList[i].health,animalList[i].hunger,animalList[i].thirst,animalList[i].sleepy);
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
	for(uint i=0;i<animalCount;i++){
		if(e == &animalList[i])                                    {continue;}
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

void animalCheckSuffocation(animal *e){
	const uint8_t cb = worldGetB(e->x,e->y,e->z);
	if(cb != 0){
		if(rngValM(128) == 0){e->health--;}
		const blockCategory cc = blockTypeGetCat(cb);
		if(cc == LEAVES){
			if(rngValM( 512) == 0){worldBoxMine(e->x,e->y,e->z,1,1,1);}
		}else if(cc == DIRT){
			if(rngValM(4096) == 0){worldBoxMine(e->x,e->y,e->z,1,1,1);}
		}
	}
}

void animalCheckForHillOrCliff(animal *e){
	vec caDir = vecNew(e->gvx,0.f,e->gvz);
	caDir = vecAdd(vecNew(e->x,e->y,e->z),vecMulS(vecNorm(caDir),0.5f));
	const uint8_t cb = worldGetB(caDir.x,caDir.y,caDir.z);
	const uint8_t ub = worldGetB(caDir.x,caDir.y+1,caDir.z);
	if((cb != 0) && (ub == 0) && (fabsf(e->vy)<0.01f)){
		if(!(e->flags & ANIMAL_FALLING)){
			e->vy = 0.03f;
		}
		return;
	}
	if(cb == 0){
		if((worldGetB(caDir.x,caDir.y-1,caDir.z) == 0) &&
		   (worldGetB(caDir.x,caDir.y-2,caDir.z) == 0)){
				e->vx = -e->gvx;
				e->vz = -e->gvz;
				return;
			}
	}
}

int animalCheckHeat(animal *e){
	if((e->age > 20) && (rngValM( 128) == 0)){
		animal *cAnim;
		if(animalClosestAnimal(e,&cAnim,e->type) < 192.f){
			if(cAnim->age > 20){
				e->state = ANIMAL_S_HEAT;
				return 1;
			}
		}
	}
	return 0;
}

void animalAgeing(animal *e){
	if(e->age < 125){
		if(rngValM(1<<14) == 0){e->age++;}
	}
	if(e->age > 64){
		if(rngValM(1<<16) <= (uint)(e->age-64)){e->health = 0;}
	}
}

void animalSleepyness(animal *e){
	if (rngValM( 512) == 0){e->sleepy--;}
	if((e->state != 1) && (e->sleepy < 16)){
		e->state = ANIMAL_S_SLEEP;
		return;
	}
	if((e->state == 1) && (e->sleepy <  2)){
		e->state = ANIMAL_S_SLEEP;
		return;
	}
}

void animalHunger(animal *e){
	if(rngValM(1024) == 0){
		e->hunger--;
	}
	if((e->state != 0) && (e->state == 1) && (e->hunger <  32)){
		e->state = ANIMAL_S_LOITER;
		return;
	}
}

void animalSLoiter(animal *e){
	character *cChar;
	float dist = animalClosestPlayer(e,&cChar);
	if((dist < 24.f) && (cChar != NULL)){
		if(!(e->flags & ANIMAL_FALLING)){
			e->vy = 0.03f;
			e->sleepy -= 2;
		}
		e->sleepy -= 2;
		e->state = ANIMAL_S_FLEE;
		return;
	}

	if(e->hunger < 64){
		const uint8_t cb = worldGetB(e->x,e->y-.6f,e->z);
		if((cb == 2) && (rngValM(128) == 0)){
			worldSetB(e->x,e->y-.6f,e->z,1);
			e->hunger += 48;
		}
	}
	if(animalCheckHeat(e)){return;}


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

	if(rngValM(2048) > (uint)(127-e->age)){
		e->state = ANIMAL_S_PLAYING;
		return;
	}
}

void animalSSleep(animal *e){
	character *cChar;
	e->gvx = 0;
	e->gvz = 0;

	if(e->sleepy > 120){
		e->state  = ANIMAL_S_LOITER;
		e->gpitch = 0.f;
		return;
	}else if(e->sleepy > 64){
		if(rngValM(1<<14) <= (uint)(e->sleepy-64)){
			e->state  = ANIMAL_S_LOITER;
			e->gpitch = 0.f;
			return;
		}
		float dist = animalClosestPlayer(e,&cChar);
		if((dist < 24.f) && (cChar != NULL)){
			if(!(e->flags & ANIMAL_FALLING)){
				e->vy     = 0.04f;
				e->sleepy -= 2;
			}
			e->sleepy -= 2;
			e->gpitch = 0.f;
			e->state  = ANIMAL_S_FLEE;
			return;
		}
	}else if(e->sleepy > 8){
		float dist = animalClosestPlayer(e,&cChar);
		if((dist < 9.f) && (cChar != NULL)){
			if(!(e->flags & ANIMAL_FALLING)){
				e->vy     = 0.05f;
				e->sleepy -= 4;
			}
			e->sleepy -= 2;
			e->gpitch = 0.f;
			e->state  = ANIMAL_S_FLEE;
			return;
		}
	}

	if(e->flags & ANIMAL_BELLYSLEEP){
		e->gpitch = -90.f;
	}else{
		e->gpitch =  90.f;
	}
	if(rngValM(16) == 0){
		e->sleepy++;
	}
}

void animalSHeat(animal *e){
	animal *cAnim;
	float dist = animalClosestAnimal(e,&cAnim,e->type);
	if((dist > 256.f) || (cAnim == NULL)){ e->state = 0; }

	if((e->hunger < 24) || (e->sleepy < 48)){
		e->state = ANIMAL_S_LOITER;
		return;
	}
	if((cAnim->hunger < 24) || (cAnim->sleepy < 48)){
		e->state = ANIMAL_S_LOITER;
		return;
	}

	if(dist < 2.f){
		e->state   =  ANIMAL_S_LOITER;
		e->hunger -= 16;
		e->sleepy -= 24;

		cAnim->state   = ANIMAL_S_LOITER;
		cAnim->hunger -= 16;
		cAnim->sleepy -= 24;

		cAnim = animalNew(e->x+((rngValf()*2.f)-1.f),e->y+.4f,e->z+((rngValf()*2.f)-1.f),e->type);
		cAnim->age = 1;
		return;
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
	if(rngValM(1<<10) == 0){e->hunger--;}
	if((dist > 96.f) && (cChar != NULL)){
		e->state   = ANIMAL_S_LOITER;
		e->sleepy -= 2;
		return;
	}
	if(cChar != NULL){
		vec caNorm = vecNorm(vecNew(e->x - cChar->x,0.f, e->z - cChar->z));
		vec caVel  = vecMulS(caNorm,0.03f);
		vec caRot  = vecVecToDeg(caNorm);

		e->gvx = caVel.x;
		e->gvz = caVel.z;
		e->yaw = -caRot.yaw;
	}
}

void animalSPlayful(animal *e){
	character *cChar;
	float dist = animalClosestPlayer(e,&cChar);
	if (rngValM( 192) == 0){e->sleepy--;}
	if((dist < 35.f) && (cChar != NULL)){
		e->state = ANIMAL_S_FLEE;
		return;
	}
	if(animalCheckHeat(e)){return;}

	if((rngValM( 128) == 0) && (fabsf(e->vy) < 0.001f)){
		e->vy = 0.03f;
	}
	if (rngValM( 256) == 0){
		e->gvx = 0;
		e->gvy = 0;
		e->gvz = 0;
	}
	if (rngValM( 512) == 0){
		e->gpitch = ((rngValf()*2.f)-1.f)*16.f;
	}
	if (rngValM( 512) == 0){
		e->gyaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if (rngValM( 256) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->yaw,0.f,0.f)),0.01f);
		e->gvx = dir.x;
		e->gvz = dir.z;
	}

	if (rngValM(1024) > (uint)( e->age)){
		e->state = 4;
		return;
	}
}

void animalSExist(animal *e){
	animalCheckSuffocation(e);
	animalCheckForHillOrCliff(e);
	animalAgeing(e);
	animalSleepyness(e);
	animalHunger(e);
}

inline static void animalThink(animal *e){
	animalSExist(e);
	switch(e->state){
		default:
		case ANIMAL_S_LOITER:
			animalSLoiter(e);
			break;
		case ANIMAL_S_FLEE:
			animalSFlee(e);
			break;
		case ANIMAL_S_HEAT:
			animalSHeat(e);
			break;
		case ANIMAL_S_SLEEP:
			animalSSleep(e);
			break;
		case ANIMAL_S_PLAYING:
			animalSPlayful(e);
			break;
	}

}

void animalThinkAll(){
	for(int i=animalCount-1;i>=0;--i){
		animalThink(&animalList[i]);
	}
}

void animalEmptySync(int c){
	packet *rp = &packetBuffer;

	rp->val.u[ 0] = 0;
	rp->val.u[ 1] = 0;
	rp->val.u[ 2] = 0;
	rp->val.u[ 3] = 0;

	packetQueue(rp,30,13*4,c);
}

void animalSync(int c, int i){
	packet *rp = &packetBuffer;
	animal *e  = &animalList[i];

	rp->val.c[ 0] = e->type;
	rp->val.c[ 1] = e->flags;
	rp->val.c[ 2] = e->state;
	rp->val.c[ 3] = e->age;

	rp->val.c[ 4] = e->health;
	rp->val.c[ 5] = e->hunger;
	rp->val.c[ 6] = e->thirst;
	rp->val.c[ 7] = e->sleepy;

	rp->val.u[ 2] = i;
	rp->val.u[ 3] = animalCount;

	rp->val.f[ 4] = e->x;
	rp->val.f[ 5] = e->y;
	rp->val.f[ 6] = e->z;
	rp->val.f[ 7] = e->yaw;
	rp->val.f[ 8] = e->pitch;
	rp->val.f[ 9] = e->roll;
	rp->val.f[10] = e->vx;
	rp->val.f[11] = e->vy;
	rp->val.f[12] = e->vz;

	packetQueue(rp,30,13*4,c);
}

uint animalSyncPlayer(int c, uint offset){
	const uint max = MIN((offset+ANIMALS_PER_UPDATE),animalCount);
	if(animalCount == 0){
		animalEmptySync(c);
		return offset;
	}

	for(uint i=offset;i<max;i++){
		animalSync(c,i);
	}
	offset += ANIMALS_PER_UPDATE;
	if(offset >= animalCount){offset=0;}
	return offset;
}

uint8_t *animalSave(animal *e, uint8_t *b){
	float *f = (float *)b;

	b[ 0] = 0x03;
	b[ 1] = e->flags;
	b[ 2] = e->type;
	b[ 3] = e->state;

	b[ 4] = e->health;
	b[ 5] = e->hunger;
	b[ 6] = e->thirst;
	b[ 7] = e->sleepy;

	b[ 8] = e->age;
	b[ 9] = 0;
	b[10] = 0;
	b[11] = 0;

	f[ 3] = e->x;
	f[ 4] = e->y;
	f[ 5] = e->z;
	f[ 6] = e->yaw;
	f[ 7] = e->pitch;
	f[ 8] = e->roll;
	f[ 9] = e->vx;
	f[10] = e->vy;
	f[11] = e->vz;

	return b+12*4;
}

uint8_t *animalLoad(uint8_t *b){
	float *f  = (float *)b;
	animal *e = animalNew(f[3],f[4],f[5],b[2]);
	if(e == NULL){return b+12*4;}

	e->yaw    = f[ 6];
	e->pitch  = f[ 7];
	e->roll   = f[ 8];
	e->vx     = f[ 9];
	e->vy     = f[10];
	e->vz     = f[11];

	e->flags  = b[ 1];
	e->state  = b[ 3];

	e->health = b[ 4];
	e->hunger = b[ 5];
	e->thirst = b[ 6];
	e->sleepy = b[ 7];

	e->age    = b[ 8];

	return b+12*4;
}

uint8_t *animalSaveChungus(chungus *c,uint8_t *b){
	if(c == NULL){return b;}
	for(uint i=0;i<animalCount;i++){
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
