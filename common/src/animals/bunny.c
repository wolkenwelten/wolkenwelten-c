#include "bunny.h"

#include "../game/animal.h"
#include "../game/being.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/item.h"
#include "../misc/misc.h"
#include "../network/messages.h"
#include "../mods/api_v1.h"

#include <stdio.h>
#include <math.h>

static void animalCheckHeat(animal *e,int stateChange[16]){
	animal *cAnim;
	if((e->pregnancy > 0) || (e->age < 20)){return;}
	stateChange[ANIMAL_S_HEAT] += rngValA(127);
	const float dist = animalClosestAnimal(e,&cAnim,e->type,0,0);
	if((dist > 192.f) || (cAnim->pregnancy > 0) || (cAnim->age < 20)){return;}
	stateChange[ANIMAL_S_HEAT] += 256.f - dist;
}

static void animalAgeing(animal *e,int stateChange[16]){
	if(e->age > 64){
		if(rngValA((1<<12)-1) <= (uint)(e->age-64)){e->health = 0;}
	}
	stateChange[ANIMAL_S_HEAT] -= e->age*2;
}

static void animalSleepyness(animal *e,int stateChange[16]){
	const int v = 48 - e->sleepy;
	stateChange[ANIMAL_S_SLEEP] += v*v;
}

static void animalHunger(animal *e,int stateChange[16]){
	const int v = 48 - e->hunger;
	stateChange[ANIMAL_S_FOOD_SEARCH] += v*v;
}

static void animalSLoiter(animal *e,int stateChange[16]){
	if(rngValA( 7) == 0){
		e->grot.yaw = e->rot.yaw + ((rngValf()*2.f)-1.f)*4.f;
	}
	if(rngValA(15) == 0){
		e->gvel = vecZero();
	}
	if(rngValA(31) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*10.f;
	}
	if(rngValA(63) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValA(31) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}

	stateChange[ANIMAL_S_PLAYING] += rngValA(127);
}

static void animalSSleep(animal *e,int stateChange[16]){
	e->gvel = vecZero();
	const int v = e->sleepy - 16;
	stateChange[ANIMAL_S_LOITER] += v*v;

	if(e->flags & ANIMAL_BELLYSLEEP){
		e->grot.pitch = -90.f;
	}else{
		e->grot.pitch =  90.f;
	}
	if(rngValA(3) == 0){
		e->sleepy++;
	}
	if(rngValA(63) == 0){
		if(e->health < animalGetMaxHealth(e)){e->health++;}
	}
}

static void animalSHeat(animal *e,int stateChange[16]){
	animal *cAnim;
	float dist;
	if(e->pregnancy > 0){
		stateChange[ANIMAL_S_LOITER] += rngValA(255);
	}
	if(e->flags & ANIMAL_MALE){
		dist = animalClosestAnimal(e,&cAnim,e->type,ANIMAL_MALE,0);
	}else{
		dist = animalClosestAnimal(e,&cAnim,e->type,ANIMAL_MALE,ANIMAL_MALE);
	}
	if((dist > 256.f) || (cAnim == NULL) || (cAnim->pregnancy > 0)){
		stateChange[ANIMAL_S_LOITER] += rngValA(1023);
		return;
	}

	if(dist < 2.f){
		if((cAnim->pregnancy < 0) && (e->pregnancy < 0)){
			if(e->flags & ANIMAL_MALE){
				cAnim->pregnancy = 48;
			}else{
				e->pregnancy = 48;
			}
			cAnim->sleepy  = MAX(8,cAnim->sleepy - 32);
			cAnim->hunger -= 16;
			e->sleepy      = MAX(8,cAnim->sleepy - 32);
			e->hunger     -= 16;
		}
	}
	if(cAnim == NULL){return;}
	const vec caNorm = vecNorm(vecSub(e->pos,cAnim->pos));
	const vec caVel  = vecMulS(caNorm,-0.03f);
	const vec caRot  = vecVecToDeg(caNorm);

	e->gvel.x  = caVel.x;
	e->gvel.z  = caVel.z;

	e->rot.yaw = -caRot.yaw + 180.f;
}

static void animalSFight(animal *e,int stateChange[16]){
	if(e->type != 2){
		if((rngValA(15) == 0) && !(e->flags & ANIMAL_FALLING)){
			e->vel.y = 0.03f;
		}
	}
	stateChange[ANIMAL_S_FLEE] += rngValA(127) + e->stateTicks;
}

static void animalSFlee(animal *e,int stateChange[16]){
	if(e->type != 2){
		if((rngValA(3) == 0) && !(e->flags & ANIMAL_FALLING)){
			e->vel.y = 0.01f;
		}
	}
	stateChange[ANIMAL_S_FIGHT] += rngValA(127) + e->stateTicks;
}

static void animalFightOrFlight(animal *e,int stateChange[16]){
	character *cChar;
	float dist = animalClosestPlayer(e,&cChar);

	if(e->state == ANIMAL_S_FIGHT){
		if((cChar == NULL) || (dist > 16.f)){
			stateChange[ANIMAL_S_LOITER] += 1024;
			return;
		}else{
			vec caNorm = vecNorm(vecNew(cChar->pos.x - e->pos.x,0.f, cChar->pos.z - e->pos.z));
			vec caVel  = vecMulS(caNorm,0.03f);
			vec caRot  = vecVecToDeg(caNorm);

			e->gvel.x  = caVel.x;
			e->gvel.y  = 0.f;
			e->gvel.z  = caVel.z;
			e->rot.yaw = -caRot.yaw;

			if((dist < 1.5f)){
				e->gvel.x = 0;
				e->gvel.z = 0;
				if(rngValA(7)==0){
					int dmg = 1;
					being bc = characterGetBeing(cChar);
					if(bc == 0){return;}
					if(rngValM(8)==0){dmg = 4;}
					msgBeingDamage(beingID(bc),dmg,2,bc,-1,e->pos);
				}
			}
		}
	}else if(e->state == ANIMAL_S_FLEE){
		if((cChar == NULL) || (dist > 32.f)){
			stateChange[ANIMAL_S_LOITER] += 1024;
			return;
		}else{
			vec caNorm = vecNorm(vecNew(e->pos.x - cChar->pos.x,0.f, e->pos.z - cChar->pos.z));
			vec caVel  = vecMulS(caNorm,0.03f);
			vec caRot  = vecVecToDeg(caNorm);

			e->gvel.x  = caVel.x;
			e->gvel.z  = caVel.z;
			e->rot.yaw = -caRot.yaw;
			if((dist < 3.f) && (rngValM(8)==0)){
				stateChange[ANIMAL_S_FIGHT] += 4096;
			}
		}
	}else{
		float fd = 9.f;
		if(e->state == ANIMAL_S_SLEEP){fd = 3.f;}
		if(e->state == ANIMAL_S_PLAYING){fd = 15.f;}
		if((cChar != NULL) && (dist < fd)){
			if(e->state == ANIMAL_S_SLEEP){
				stateChange[ANIMAL_S_FIGHT] += 4096;
			}else{
				stateChange[ANIMAL_S_FLEE] += 4096;
			}
			return;
		}
	}
}

static void animalSPlayful(animal *e,int stateChange[16]){
	if((rngValA(31) == 0) && !(e->flags & ANIMAL_FALLING)){
		e->vel.y = 0.03f;
	}
	if (rngValA(15) == 0){
		e->gvel = vecZero();
	}
	if (rngValA(15) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*16.f;
	}
	if (rngValA(15) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if (rngValA( 7) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(-e->rot.yaw,0.f,0.f)),0.02f);
		e->gvel.x = dir.x;
		e->gvel.y = 0.f;
		e->gvel.z = dir.z;
	}

	stateChange[ANIMAL_S_LOITER] += (e->age*4);
}

static void animalSFoodSearch(animal *e,int stateChange[16]){
	const u8 cb = worldGetB(e->pos.x,e->pos.y-1,e->pos.z);
	if(cb == 2){
		e->gvel = vecZero();
		stateChange[ANIMAL_S_EAT] += 4096;
		return;
	}
	const int v = MAX(0,e->hunger - 16);
	stateChange[ANIMAL_S_FOOD_SEARCH] -= v*v;

	if(rngValA(3) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*16.f;
	}
	if(rngValA(3) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValA(7) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(-e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}
	if(rngValA(15) == 0){
		e->gvel = vecZero();
	}
}

static void animalSEat(animal *e,int stateChange[16]){
	const u8 cb = worldGetB(e->pos.x,(int)e->pos.y-1,e->pos.z);
	if(cb != 2){
		stateChange[ANIMAL_S_EAT] -= 8192;
		return;
	}
	const int v = MAX(0,e->hunger - 16);
	stateChange[ANIMAL_S_EAT] -= v*v;

	if(rngValA(  7) == 0){e->hunger++;}
	if(rngValA( 31) == 0){
		if(e->health < animalGetMaxHealth(e)){e->health++;}
	}
	if(rngValA(127) == 0){
		worldSetB(e->pos.x,(int)e->pos.y-1,e->pos.z,1);
	}
}

static void animalDoPoop(animal *e,int stateChange[16]){
	item ipoop = itemNew(I_Poop,1);
	itemDropNewP(e->pos, &ipoop);
	stateChange[ANIMAL_S_FLEE] += 256;
}

static void animalPregnancy(animal *e,int stateChange[16]){
	if(e->flags & ANIMAL_MALE){e->pregnancy = -1; return;}
	if(e->pregnancy  <  0)    {return;}
	if(e->age        < 21)    {e->pregnancy = -1;}

	if(e->pregnancy == 0){
		animal *cAnim = animalNew(vecNew(e->pos.x+((rngValf()*2.f)-1.f),e->pos.y+.4f,e->pos.z+((rngValf()*2.f)-1.f)),e->type,-1);
		if(cAnim != NULL){
			cAnim->age = 1;
		}
		e->hunger   -= 8;
		e->sleepy    = MAX(8,e->sleepy-24);
		e->pregnancy = -1;
		animalDoPoop(e,stateChange);
	}
	(void)stateChange;
}

static void animalPoop(animal *e,int stateChange[16]){
	if(e->hunger < 24){return;}
	if(e->hunger < 48){
		if(rngValA(2047) == 0){animalDoPoop(e,stateChange);}
	}else{
		if(rngValA(1023) == 0){animalDoPoop(e,stateChange);}
	}
}

static void animalSocialDistancing(animal *e,int stateChange[16]){
	if(rngValA(31) != 0){return;}
	for(uint i=0;i<animalCount;i++){
		if(animalList[i].type == 0)                     {continue;}
		if(e == &animalList[i])                         {continue;}
		if(fabsf(e->pos.x - animalList[i].pos.x) > 1.f) {continue;}
		if(fabsf(e->pos.y - animalList[i].pos.y) > 1.f) {continue;}
		if(fabsf(e->pos.z - animalList[i].pos.z) > 1.f) {continue;}
		e->rot.yaw = ((rngValf()*2.f)-1.f)*360.f;
		vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
		if(!(e->flags & ANIMAL_FALLING)){
			e->vel.y  = 0.03f;
		}
		stateChange[ANIMAL_S_FLEE] += 256;
		return;
	}
}

static void animalStateChange(animal *e,int stateChange[16]){
	uint max=0;

	stateChange[e->state] -= e->stateTicks;
	stateChange[e->state] *= stateChange[e->state];
	for(uint i=1;i<16;i++){
		if(stateChange[i] > stateChange[max]){max = i;}
	}
	if(e->state == max){return;}
	e->stateTicks = 0;
	if(e->state == ANIMAL_S_SLEEP){
		e->grot.pitch = 0.f;
		if(!(e->flags & ANIMAL_FALLING)){
			if((max == ANIMAL_S_FIGHT) || (max == ANIMAL_S_FLEE)){
				//e->vel.y   = 0.04f;
			}
		}
	}
	e->state = max;
}

void animalThinkBunny(animal *e){
	static int stateChange[16];
	for(uint i=0;i<16;i++){
		stateChange[i] = rngValA(127);
	}
	e->stateTicks++;
	animalCheckHeat       (e,stateChange);
	animalCheckSuffocation(e);
	if(0){animalFightOrFlight(e,stateChange);}
	animalAgeing          (e,stateChange);
	animalSleepyness      (e,stateChange);
	animalHunger          (e,stateChange);
	animalPregnancy       (e,stateChange);
	animalPoop            (e,stateChange);
	animalSocialDistancing(e,stateChange);

	switch(e->state){
	default:
	case ANIMAL_S_LOITER:
		animalSLoiter(e,stateChange);
		break;
	case ANIMAL_S_FLEE:
		animalSFlee(e,stateChange);
		break;
	case ANIMAL_S_HEAT:
		animalSHeat(e,stateChange);
		break;
	case ANIMAL_S_SLEEP:
		animalSSleep(e,stateChange);
		break;
	case ANIMAL_S_PLAYING:
		animalSPlayful(e,stateChange);
		break;
	case ANIMAL_S_FOOD_SEARCH:
		animalSFoodSearch(e,stateChange);
		break;
	case ANIMAL_S_EAT:
		animalSEat(e,stateChange);
		break;
	case ANIMAL_S_FIGHT:
		animalSFight(e,stateChange);
		break;
	}
	animalStateChange(e,stateChange);
}

void animalRDieBunny(animal *e){
	item mdrop = itemNew(I_Meat,rngValMM(2,4));
	item fdrop = itemNew(I_Fur, rngValMM(1,2));
	itemDropNewP(e->pos,&mdrop);
	itemDropNewP(e->pos,&fdrop);
}
