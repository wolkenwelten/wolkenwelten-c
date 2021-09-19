/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bunny.h"

#include "../game/animal.h"
#include "../game/being.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/item.h"
#include "../game/itemDrop.h"
#include "../game/time.h"
#include "../misc/misc.h"
#include "../network/messages.h"
#include "../world/world.h"

#include <stdio.h>
#include <math.h>

static void bunnyCheckHeat(animal *e,int stateChange[16]){
	animal *cAnim;
	float d;
	if((e->pregnancy > 0) || (e->age < 20)){return;}
	stateChange[ANIMAL_S_HEAT] += rngValA(127);
	being oa = beingListGetClosest(beingListGet(e->pos.x,e->pos.y,e->pos.z), animalGetBeing(e), BEING_ANIMAL, &d);
	if(oa == 0){return;}
	cAnim = animalGetByBeing(oa);
	if((d > 192.f) || (cAnim->pregnancy > 0) || (cAnim->age < 20)){return;}
	stateChange[ANIMAL_S_HEAT] += (256.f - d)*4.f;
}

static void bunnyAgeing(animal *e,int stateChange[16]){
	if(e->age > 64){
		if(rngValA((1<<12)-1) <= (uint)(e->age-64)){e->health = 0;}
	}
	stateChange[ANIMAL_S_HEAT] -= e->age*2;
}

static void bunnySleepyness(animal *e,int stateChange[16]){
	const int v = 48 - e->sleepy;
	stateChange[ANIMAL_S_SLEEP] += v*v;
}

static void bunnyHunger(animal *e,int stateChange[16]){
	const int v = 48 - e->hunger;
	stateChange[ANIMAL_S_FOOD_SEARCH] += v*v;
}

static void bunnySLoiter(animal *e,int stateChange[16]){
	if(rngValA( 7) == 0){
		e->grot.yaw = e->rot.yaw + ((rngValf()*2.f)-1.f)*4.f;
		e->gvel = vecZero();
	}
	if(rngValA(15) == 0){
		e->gvel = vecZero();
	}
	if(rngValA(31) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*10.f;
	}
	if(rngValA(63) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
		e->gvel = vecZero();
	}
	if(rngValA(31) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}

	stateChange[ANIMAL_S_PLAYING] += rngValA(127);
}

static void bunnySSleep(animal *e,int stateChange[16]){
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

static void bunnySHeat(animal *e,int stateChange[16]){
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

static void bunnySFight(animal *e,int stateChange[16]){
	e->grot.pitch = 0.f;
	if(e->type != 2){
		if((rngValA(15) == 0) && !(e->flags & ANIMAL_FALLING)){
			e->vel.y = 0.03f;
		}
	}
	stateChange[ANIMAL_S_FLEE] += rngValA(127) + e->stateTicks;
}

static void bunnySFlee(animal *e,int stateChange[16]){
	e->grot.pitch = 0.f;
	if(e->type != 2){
		if((fabsf(e->gvel.x) + fabsf(e->gvel.z)) < 0.02f){
			vec dir = vecMulS(vecDegToVec(vecNew(-e->rot.yaw,0.f,0.f)),0.03f);
			e->gvel.x = dir.x;
			e->gvel.y = 0.f;
			e->gvel.z = dir.z;
		}
		if((rngValA(3) == 0) && !(e->flags & ANIMAL_FALLING)){
			e->vel.y = 0.01f;
		}
	}
	stateChange[ANIMAL_S_FIGHT] += rngValA(127) + e->stateTicks;
}

static void bunnyFightOrFlight(animal *e,int stateChange[16]){
	if(e->state == ANIMAL_S_FIGHT){
		if(e->target == 0){e->target = animalFindFOFTarget(e);}
		if(e->target != 0){
			const vec tpos = beingGetPos(e->target);
			const float dist = vecMag(vecSub(tpos,e->pos));

			if((dist < 1.5f) && (rngValA(7)==0)){
				int dmg = 1;
				if(rngValM(8)==0){dmg = 4;}
				beingDamage(e->target,dmg,2,1.f,animalGetBeing(e),e->pos);
			}else if(dist > 16){
				stateChange[ANIMAL_S_LOITER] += (dist-16)*2048;
			}
		}else{
			stateChange[ANIMAL_S_LOITER] += 8192;
		}
	/* TODO: Fleeing without a cchar */
	}else if(e->state == ANIMAL_S_FLEE){
		if(e->target == 0){e->target = animalFindFOFTarget(e);}
		if(e->target != 0){
			const vec tpos = beingGetPos(e->target);
			const float dist = vecMag(vecSub(tpos,e->pos));

			if(dist < 6.f){
				stateChange[ANIMAL_S_FIGHT] += (1 << 14) * (dist - 6.f);
			}else if(dist > 48.f){
				stateChange[ANIMAL_S_LOITER] += (dist-48)*4096;
			}
		}else{
			stateChange[ANIMAL_S_LOITER] += 8192;
		}
	}else{
		character *cChar;
		float dist = animalClosestPlayer(e,&cChar);
		float fd = 9.f;
		if(e->state == ANIMAL_S_SLEEP){fd = 3.f;}
		if(e->state == ANIMAL_S_PLAYING){fd = 15.f;}
		if((cChar != NULL) && (dist < fd)){
			if(e->state == ANIMAL_S_SLEEP){
				stateChange[ANIMAL_S_FIGHT] += (1<<14) * (fd - dist) * (fd - dist);
			}else{
				stateChange[ANIMAL_S_FLEE] += (1<<14) * (fd - dist) * (fd - dist);
			}
			return;
		}
	}
}

static void bunnySPlayful(animal *e,int stateChange[16]){
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

static void bunnySFoodSearch(animal *e,int stateChange[16]){
	const u8 cb = worldGetB(e->pos.x,e->pos.y-1,e->pos.z);
	const u8 tb = worldGetB(e->pos.x,e->pos.y-2,e->pos.z);
	if((cb == I_Grass) || (tb == I_Grass)){
		e->gvel = vecZero();
		stateChange[ANIMAL_S_EAT] += 1<<20;
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

static void bunnySEat(animal *e,int stateChange[16]){
	const u8 cb = worldGetB(e->pos.x,(int)e->pos.y-1,e->pos.z);
	const u8 tb = worldGetB(e->pos.x,(int)e->pos.y-2,e->pos.z);
	if((cb != I_Grass) && (tb != I_Grass)){
		stateChange[ANIMAL_S_EAT] -= 1<<16;
		return;
	}
	const int v = MAX(1,e->hunger - 48);
	stateChange[ANIMAL_S_EAT] -= v*v;

	if(rngValA(  7) == 0){e->hunger++;}
	if(rngValA( 31) == 0){
		if(e->health < animalGetMaxHealth(e)){e->health++;}
	}
	if(rngValA(127) == 0){
		worldSetB(e->pos.x,(int)e->pos.y - cb == I_Grass ? -1 : -2,e->pos.z,1);
	}
}

static void bunnyDoPoop(animal *e,int stateChange[16]){
	item ipoop = itemNew(I_Poop,1);
	itemDropNewP(e->pos, &ipoop,-1);
	stateChange[ANIMAL_S_FLEE] += 256;
}

static void bunnyPregnancy(animal *e,int stateChange[16]){
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
		bunnyDoPoop(e,stateChange);
	}
	(void)stateChange;
}

static void bunnyPoop(animal *e,int stateChange[16]){
	if(e->hunger < 24){return;}
	if(e->hunger < 48){
		if(rngValA(2047) == 0){bunnyDoPoop(e,stateChange);}
	}else{
		if(rngValA(1023) == 0){bunnyDoPoop(e,stateChange);}
	}
}

static void bunnySocialDistancing(animal *e,int stateChange[16]){
	if(rngValA(15) != 0){return;}
	float d;
	being ca = beingListGetClosest(e->bl, animalGetBeing(e), BEING_ANIMAL, &d);
	if((ca == 0) || (d > 1)){return;}

	animal *oa = animalGetByBeing(ca);
	if((e->flags & ANIMAL_MALE) && (oa->flags & ANIMAL_MALE)){
		e->state = ANIMAL_S_FIGHT;
		e->target = ca;
	}
	e->rot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
	e->gvel.x = dir.x;
	e->gvel.z = dir.z;
	if(!(e->flags & ANIMAL_FALLING)){
		e->vel.y  = 0.03f;
	}
	stateChange[ANIMAL_S_FLEE] += 256;
}

static void bunnyStateChange(animal *e,int stateChange[16]){
	uint max=0;

	stateChange[e->state] += 512;
	stateChange[e->state] -= e->stateTicks;
	if(stateChange[e->state] > 0){
		stateChange[e->state] *= stateChange[e->state];
	}
	for(uint i=1;i<16;i++){
		if(stateChange[i] > stateChange[max]){max = i;}
	}
	if(e->state == max){return;}
	e->stateTicks = 0;
	if(e->state == ANIMAL_S_SLEEP){
		e->grot.pitch = 0.f;
		if(!(e->flags & ANIMAL_FALLING)){
			if((max == ANIMAL_S_FIGHT) || (max == ANIMAL_S_FLEE)){
				e->vel.y = 0.04f;
			}
		}
	}
	e->state = max;
}

void animalThinkBunny(animal *e){
	static int stateChange[16];
	for(uint i=0;i<16;i++){
		stateChange[i] = rngValA(63);
	}
	stateChange[ANIMAL_S_FIGHT] = 0;
	stateChange[ANIMAL_S_FLEE]  = 0;

	e->stateTicks++;
	animalCheckSuffocation(e);
	bunnyCheckHeat       (e,stateChange);
	bunnyFightOrFlight   (e,stateChange);
	bunnyAgeing          (e,stateChange);
	bunnySleepyness      (e,stateChange);
	bunnyHunger          (e,stateChange);
	bunnyPregnancy       (e,stateChange);
	bunnyPoop            (e,stateChange);
	bunnySocialDistancing(e,stateChange);

	int tcat = gtimeGetTimeCat();
	if((tcat == TIME_NIGHT) || (tcat == TIME_EVENING)){
		stateChange[ANIMAL_S_SLEEP] += rngValA(255);
		stateChange[ANIMAL_S_SLEEP] *= 16;
	}else{
		stateChange[ANIMAL_S_SLEEP]  = MAX(1,stateChange[ANIMAL_S_SLEEP]);
		stateChange[ANIMAL_S_SLEEP] /= 16;
	}

	switch(e->state){
	default:
	case ANIMAL_S_LOITER:
		bunnySLoiter(e,stateChange);
		break;
	case ANIMAL_S_FLEE:
		bunnySFlee(e,stateChange);
		break;
	case ANIMAL_S_HEAT:
		bunnySHeat(e,stateChange);
		break;
	case ANIMAL_S_SLEEP:
		bunnySSleep(e,stateChange);
		break;
	case ANIMAL_S_PLAYING:
		bunnySPlayful(e,stateChange);
		break;
	case ANIMAL_S_FOOD_SEARCH:
		bunnySFoodSearch(e,stateChange);
		break;
	case ANIMAL_S_EAT:
		bunnySEat(e,stateChange);
		break;
	case ANIMAL_S_FIGHT:
		bunnySFight(e,stateChange);
		break;
	}
	bunnyStateChange(e,stateChange);
}

void animalRDieBunny(animal *e){
	item mdrop = itemNew(I_Meat,rngValMM(2,4));
	item fdrop = itemNew(I_Fur, rngValMM(1,2));
	itemDropNewP(e->pos,&mdrop,-1);
	itemDropNewP(e->pos,&fdrop,-1);
}

void animalRBurnBunny(animal *e){
	e->state = ANIMAL_S_FLEE;
	if(!(e->flags & ANIMAL_FALLING)){
		e->vel.y = 0.04f;
	}
}
