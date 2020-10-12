#include "animal.h"

#include "../game/itemDrop.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>
#include <math.h>

animal  animalList[1<<10];
uint    animalCount = 0;
animal *animalFirstFree = NULL;

#define ANIMALS_PER_UPDATE 16u

animal *animalNew(const vec pos , int type){
	animal *e = NULL;
	if(animalCount >= ((sizeof(animalList) / sizeof(animal)-1))){return NULL;}
	e = &animalList[animalCount++];
	animalReset(e);

	e->pos       = pos;

	e->age       = 21;
	e->health    =  8;
	e->hunger    = 64;
	e->thirst    = 64;
	e->sleepy    = 64;

	e->type      = type;

	if(rngValM(2) == 0){
		e->flags |= ANIMAL_BELLYSLEEP;
	}
	if(rngValM(2) == 0){
		e->flags |= ANIMAL_AGGRESIVE;
	}

	return e;
}

static void animalDel(uint i){
	if(i >= animalCount) {return;}
	animalList[i] = animalList[--animalCount];
}


static void animalRDie(animal *e){
	item drop = itemNew(I_Pear,rngValMM(3,6));
	itemDropNewP(e->pos,&drop);
}

void animalUpdateAll(){
	for(int i=animalCount-1;i>=0;i--){
		int dmg = animalUpdate(&animalList[i]);
		animalList[i].health -= dmg;
		if((animalList[i].pos.y  < -256.f) ||
		   (animalList[i].health < 0) ||
		   (animalList[i].hunger < 0) ||
		   (animalList[i].thirst < 0) ||
		   (animalList[i].sleepy < 0)) {
			fprintf(stderr,"Dead Animal [HP: %i | HUN: %i | THI: %i | SLP: %i]\n",animalList[i].health,animalList[i].hunger,animalList[i].thirst,animalList[i].sleepy);
			animalRDie(&animalList[i]);
			animalDel(i);
			continue;
		}
	}
}

float animalClosestPlayer(animal *e, character **cChar){
	*cChar = NULL;
	float ret = 4096.f;
	for(uint i=0;i<clientCount;++i){
		if(clients[i].c == NULL){continue;}
		const float d = vecMag(vecSub(clients[i].c->pos,e->pos));
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
		const float d = vecMag(vecSub(animalList[i].pos,e->pos));
		if(d < ret){
			ret = d;
			*cAnim = &animalList[i];
		}
	}
	return ret;
}

void animalCheckSuffocation(animal *e){
	const u8 cb = worldGetB(e->pos.x,e->pos.y,e->pos.z);
	if(cb != 0){
		if(rngValM(128) == 0){e->health--;}
		const blockCategory cc = blockTypeGetCat(cb);
		if(cc == LEAVES){
			if(rngValM( 512) == 0){worldBoxMine(e->pos.x,e->pos.y,e->pos.z,1,1,1);}
		}else if(cc == DIRT){
			if(rngValM(4096) == 0){worldBoxMine(e->pos.x,e->pos.y,e->pos.z,1,1,1);}
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
		if(rngValM(1<<8) == 0){e->age++;}
	}
	if(e->age > 64){
		if(rngValM(1<<12) <= (uint)(e->age-64)){e->health = 0;}
	}
}

void animalSleepyness(animal *e){
	if (rngValM( 32) == 0){e->sleepy--;}
	if((e->state != ANIMAL_S_FLEE) && (e->state != ANIMAL_S_FIGHT) && (e->sleepy < 16)){
		e->state = ANIMAL_S_SLEEP;
		return;
	}
	if(e->sleepy <  8){
		e->state = ANIMAL_S_SLEEP;
		return;
	}
}

void animalHunger(animal *e){
	if(rngValM(32) == 0){
		e->hunger--;
	}
	if(e->state == ANIMAL_S_FOOD_SEARCH){return;}
	if(e->state == ANIMAL_S_EAT){return;}
	if(e->hunger < (int)rngValM(40)){
		e->state = ANIMAL_S_FOOD_SEARCH;
		return;
	}
}

void animalSLoiter(animal *e){
	if(e->hunger < 64){
		const u8 cb = worldGetB(e->pos.x,e->pos.y-.6f,e->pos.z);
		if((cb == 2) && (rngValM(128) == 0)){
			worldSetB(e->pos.x,e->pos.y-.6f,e->pos.z,1);
			e->hunger += 48;
		}
	}
	if(animalCheckHeat(e)){return;}

	if(rngValM( 8) == 0){
		e->grot.yaw = e->rot.yaw + ((rngValf()*2.f)-1.f)*4.f;
	}
	if(rngValM(16) == 0){
		e->gvel = vecZero();
	}
	if(rngValM(48) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*10.f;
	}
	if(rngValM(64) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValM(32) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}

	if(rngValM(256) < (uint)(127-e->age)){
		e->state = ANIMAL_S_PLAYING;
		return;
	}
}

void animalSSleep(animal *e){
	e->gvel = vecZero();

	if(e->sleepy > 120){
		e->state      = ANIMAL_S_LOITER;
		e->grot.pitch = 0.f;
		return;
	}else if(e->sleepy > 64){
		if(rngValM(1<<9) <= (uint)(e->sleepy-64)*2){
			e->state      = ANIMAL_S_LOITER;
			e->grot.pitch = 0.f;
			return;
		}
	}

	if(e->flags & ANIMAL_BELLYSLEEP){
		e->grot.pitch = -90.f;
	}else{
		e->grot.pitch =  90.f;
	}
	if(rngValM(4) == 0){
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
	if((cAnim != NULL) && ((cAnim->hunger < 24) || (cAnim->sleepy < 48))){
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

		cAnim = animalNew(vecNew(e->pos.x+((rngValf()*2.f)-1.f),e->pos.y+.4f,e->pos.z+((rngValf()*2.f)-1.f)),e->type);
		if(cAnim != NULL){
			cAnim->age = 1;
		}
		return;
	}
	if(cAnim == NULL){return;}
	const vec caNorm = vecNorm(vecSub(e->pos,cAnim->pos));
	const vec caVel  = vecMulS(caNorm,-0.03f);
	const vec caRot  = vecVecToDeg(caNorm);

	e->gvel.x  = caVel.x;
	e->gvel.z  = caVel.z;

	e->rot.yaw = -caRot.yaw + 180.f;
}

void animalSFight(animal *e){
	if(rngValM(32) == 0){e->hunger--;}
	if(rngValM(24) == 0){e->sleepy--;}
}


void animalSFlee(animal *e){
	if(rngValM(32) == 0){e->hunger--;}
	if(rngValM(24) == 0){e->sleepy--;}
}

void animalCheckForCharacter(animal *e){
	character *cChar;
	float dist = animalClosestPlayer(e,&cChar);

	if(e->state == ANIMAL_S_FIGHT){
		if((cChar == NULL) || (dist > 16.f)){
			e->state   =  ANIMAL_S_LOITER;
			e->flags  &= ~ANIMAL_AGGRESIVE;
			return;
		}else{
			vec caNorm = vecNorm(vecNew(cChar->pos.x - e->pos.x,0.f, cChar->pos.z - e->pos.z));
			vec caVel  = vecMulS(caNorm,0.03f);
			vec caRot  = vecVecToDeg(caNorm);

			e->gvel.x  = caVel.x;
			e->gvel.z  = caVel.z;
			e->rot.yaw = caRot.yaw;

			if((dist < 2.f) && (rngValM(4)==0)){
				int target = getClientByCharacter(cChar);
				int dmg = 1;
				if(target < 0){return;}
				if(rngValM(8)==0){dmg = 4;}
				msgPlayerDamage(target,dmg,target,2,-1,e->pos);
			}
		}
	}else if(e->state == ANIMAL_S_FLEE){
		if((cChar == NULL) || (dist > 32.f)){
			e->state = ANIMAL_S_LOITER;
			return;
		}else{
			vec caNorm = vecNorm(vecNew(e->pos.x - cChar->pos.x,0.f, e->pos.z - cChar->pos.z));
			vec caVel  = vecMulS(caNorm,0.03f);
			vec caRot  = vecVecToDeg(caNorm);

			e->gvel.x  = caVel.x;
			e->gvel.z  = caVel.z;
			e->rot.yaw = -caRot.yaw;
			if(dist < 2.f){
				e->state = ANIMAL_S_FIGHT;
				e->flags |= ANIMAL_AGGRESIVE;
			}
		}
	}else{
		float fd = 9.f;
		if(e->state == ANIMAL_S_SLEEP){fd = 3.f;}
		if(e->state == ANIMAL_S_PLAYING){fd = 15.f;}
		if((cChar != NULL) && (dist < fd)){
			if((e->state == ANIMAL_S_SLEEP) && !(e->flags & ANIMAL_FALLING)){
				e->vel.y   = 0.04f;
			}
			e->grot.pitch = 0.f;
			if(e->state == ANIMAL_S_SLEEP){
				e->state = ANIMAL_S_FIGHT;
				e->flags |= ANIMAL_AGGRESIVE;
			}else{
				e->state = ANIMAL_S_FLEE;
			}
			return;
		}
	}
}

void animalSPlayful(animal *e){
	if (rngValM( 8) == 0){e->sleepy--;}
	if(animalCheckHeat(e)){return;}

	if((rngValM( 8) == 0) && !(e->flags & ANIMAL_FALLING)){
		e->vel.y = 0.03f;
	}
	if (rngValM(12) == 0){
		e->gvel = vecZero();
	}
	if (rngValM(16) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*16.f;
	}
	if (rngValM(16) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if (rngValM(24) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}

	if (rngValM(1024) < (uint)(e->age*6)){
		e->state = ANIMAL_S_LOITER;
		return;
	}
}

void animalSFoodSearch(animal *e){
	const u8 cb = worldGetB(e->pos.x,e->pos.y,e->pos.z);
	if(cb == 2){
		e->gvel = vecZero();
		e->state = ANIMAL_S_EAT;
	}
	if((int)rngValM(178) > (e->hunger-24)){
		e->state = ANIMAL_S_LOITER;
	}

	if(rngValM(4) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*16.f;
	}
	if(rngValM(4) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValM(6) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}
	if(rngValM(16) == 0){
		e->gvel = vecZero();
	}
}

void animalSEat(animal *e){
	const u8 cb = worldGetB(e->pos.x,e->pos.y,e->pos.z);
	if(cb != 2){
		e->state = ANIMAL_S_FOOD_SEARCH;
	}
	if(rngValM(8) == 0){
		e->hunger++;
	}
	if(rngValM(32) == 0){
		if(e->health < 8){e->health++;}
	}
	if(rngValM(64) == 0){
		worldSetB(e->pos.x,e->pos.y,e->pos.z,1);
	}
}

void animalSExist(animal *e){
	animalCheckSuffocation(e);
	animalCheckForCharacter(e);
	animalAgeing(e);
	animalSleepyness(e);
	animalHunger(e);
}

static void animalRHit(animal *e){
	if(fabsf(e->vel.y) < 0.001f){
		e->vel.y = 0.03f;
	}
	if(e->state == ANIMAL_S_FLEE || (e->flags & ANIMAL_AGGRESIVE)){
		e->state = ANIMAL_S_FIGHT;
		e->flags |= ANIMAL_AGGRESIVE;
	}else{
		e->state = ANIMAL_S_FLEE;
	}
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
	case ANIMAL_S_FOOD_SEARCH:
		animalSFoodSearch(e);
		break;
	case ANIMAL_S_EAT:
		animalSEat(e);
		break;
	case ANIMAL_S_FIGHT:
		animalSFight(e);
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

	packetQueue(rp,30,18*4,c);
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

	rp->val.f[ 4] = e->pos.x;
	rp->val.f[ 5] = e->pos.y;
	rp->val.f[ 6] = e->pos.z;
	rp->val.f[ 7] = e->vel.x;
	rp->val.f[ 8] = e->vel.y;
	rp->val.f[ 9] = e->vel.z;
	rp->val.f[10] = e->gvel.x;
	rp->val.f[11] = e->gvel.y;
	rp->val.f[12] = e->gvel.z;
	rp->val.f[13] = e->rot.yaw;
	rp->val.f[14] = e->rot.pitch;
	rp->val.f[15] = e->grot.yaw;
	rp->val.f[16] = e->grot.pitch;

	packetQueue(rp,30,17*4,c);
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

void animalDelChungus(const chungus *c){
	if(c == NULL){return;}
	for(int i=animalCount-1;i>=0;i--){
		if(animalList[i].curChungus != c){continue;}
		animalDel(i);
	}
}

void animalDmgPacket(int c, const packet *p){
	(void)c;
	i16 dmg = p->val.s[0];
	u16 i   = p->val.s[1];
	if(i >= animalCount){return;}
	animalList[i].health -= dmg;
	if(animalList[i].health <= 0){
		animalRDie(&animalList[i]);
		animalDel(i);
	}else{
		animalRHit(&animalList[i]);
	}
}
