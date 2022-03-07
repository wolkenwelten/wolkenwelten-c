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

static void werebunnyAgeing(animal *e,int stateChange[16]){
	if(e->age > 64){
		if(rngValA((1<<12)-1) <= (uint)(e->age-64)){e->health = 0;}
	}
	stateChange[ANIMAL_S_HEAT] -= e->age*2;
}

static void werebunnySleepyness(animal *e,int stateChange[16]){
	const int v = 48 - e->sleepy;
	stateChange[ANIMAL_S_SLEEP] += v*v;
}

static void werebunnyHunger(animal *e,int stateChange[16]){
	const int v = 48 - e->hunger;
	stateChange[ANIMAL_S_FOOD_SEARCH] += v*v;
}

static void werebunnySLoiter(animal *e,int stateChange[16]){
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

static void werebunnySSleep(animal *e,int stateChange[16]){
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

static void werebunnySHunt(animal *e,int stateChange[16]){
	if(e->target == 0){
		e->state = ANIMAL_S_LOITER;
		return;
	}else if(e->target != 0){
		const vec tpos = beingGetPos(e->target);
		const float dist = vecMag(vecSub(tpos,e->pos));

		if(dist < 4.f){
			int dmg = 1;
			if(rngValM(8)==0){dmg = 4;}
			beingDamage(e->target,dmg,2,1.f,animalGetBeing(e),e->pos);
			e->hunger  = MIN(e->hunger+48,127);
			e->sleepy -= 8;
			e->target  = 0;
			e->state   = ANIMAL_S_LOITER;

			animal *oa = animalGetByBeing(e->target);
			if(oa != NULL){
				e->type = 3;
			}
		}else if(dist > 32){
			stateChange[ANIMAL_S_LOITER] += (dist-32)*4096;
		}
	}else{
		stateChange[ANIMAL_S_LOITER] += 8192;
	}
}

static void werebunnySFight(animal *e,int stateChange[16]){
	if(e->type != 2){
		if((rngValA(15) == 0) && !(e->flags & ANIMAL_FALLING)){
			e->vel.y = 0.03f;
		}
	}
	stateChange[ANIMAL_S_FLEE] += rngValA(127) + e->stateTicks;
}

static void werebunnySFlee(animal *e,int stateChange[16]){
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

static void werebunnyFightOrFlightDay(animal *e,int stateChange[16]){
	if(e->state == ANIMAL_S_FIGHT){
		if(e->target == 0){e->target = animalFindFOFTarget(e);}
		if(e->target != 0){
			const vec tpos = beingGetPos(e->target);
			const float dist = vecMag(vecSub(tpos,e->pos));

			if((dist < 1.5f) && (rngValA(7)==0)){
				int dmg = 1;
				if(rngValM(8)==0){dmg = 4;}
				beingDamage(e->target,dmg,2,1.f,animalGetBeing(e),e->pos);
			}else if(dist > 32){
				stateChange[ANIMAL_S_LOITER] += (dist-32)*2048;
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
		float fd = 16.f;
		if(e->state == ANIMAL_S_SLEEP)  {fd = 3.f;}
		if(e->state == ANIMAL_S_PLAYING){fd = 9.f;}
		if((cChar != NULL) && (dist < fd)){
			stateChange[ANIMAL_S_FIGHT] += 8192 * (dist - fd);
			return;
		}
	}
}

static void werebunnyFightOrFlightNight(animal *e,int stateChange[16]){
	if(e->state == ANIMAL_S_FIGHT){
		if(e->target == 0){e->target = animalFindFOFTarget(e);}
		if(e->target != 0){
			const vec tpos = beingGetPos(e->target);
			const float dist = vecMag(vecSub(tpos,e->pos));

			if((dist < 2.5f) && (rngValA(7)==0)){
				int dmg = 1;
				if(rngValM(8)==0){dmg = 4;}
				beingDamage(e->target,dmg,2,3.f,animalGetBeing(e),e->pos);
			}else if(dist > 40){
				stateChange[ANIMAL_S_LOITER] += (dist-40)*2048;
			}
		}else{
			stateChange[ANIMAL_S_LOITER] += 8192;
		}
	}else{
		character *cChar;
		float dist = animalClosestPlayer(e,&cChar);
		float fd = 24.f;
		if(e->state == ANIMAL_S_SLEEP){fd = 3.f;}
		if(e->state == ANIMAL_S_PLAYING){fd = 15.f;}
		if((cChar != NULL) && (dist < fd)){
			stateChange[ANIMAL_S_FIGHT] += 8192 * (dist - fd) * (dist - fd);
			return;
		}
	}
}

static void werebunnyFightOrFlight(animal *e, int stateChange[16]){
	float sunlight = gtimeGetBrightness(gtimeGetTimeOfDay());
	if(sunlight < 0.5f){
		werebunnyFightOrFlightNight(e,stateChange);
	}else{
		werebunnyFightOrFlightDay(e,stateChange);
	}
}

static void werebunnySPlayful(animal *e,int stateChange[16]){
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

static void werebunnySFoodSearch(animal *e,int stateChange[16]){
	const u8 cb = worldTryB(e->pos.x,e->pos.y-1,e->pos.z);
	if(cb == 2){
		e->gvel = vecZero();
		stateChange[ANIMAL_S_EAT] += 1<<16;
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

static void werebunnySEat(animal *e,int stateChange[16]){
	const u8 cb = worldTryB(e->pos.x,(int)e->pos.y-1,e->pos.z);
	if(cb != 2){
		stateChange[ANIMAL_S_EAT] -= 1<<16;
		return;
	}
	const int v = MAX(1,e->hunger - 16);
	stateChange[ANIMAL_S_EAT] -= v*v;

	if(rngValA(  7) == 0){e->hunger++;}
	if(rngValA( 31) == 0){
		if(e->health < animalGetMaxHealth(e)){e->health++;}
	}
	if(rngValA(127) == 0){
		worldSetB(e->pos.x,(int)e->pos.y-1,e->pos.z,1);
	}
}

static void werebunnyDoPoop(animal *e,int stateChange[16]){
	item ipoop = itemNew(I_Poop,1);
	itemDropNewP(e->pos, &ipoop,-1);
	stateChange[ANIMAL_S_FLEE] += 256;
}

static void werebunnyPoop(animal *e,int stateChange[16]){
	if(e->hunger < 24){return;}
	if(e->hunger < 48){
		if(rngValA(2047) == 0){werebunnyDoPoop(e,stateChange);}
	}else{
		if(rngValA(1023) == 0){werebunnyDoPoop(e,stateChange);}
	}
}

static void werebunnySocialDistancing(animal *e,int stateChange[16]){
	if(rngValA(31) != 0){return;}
	float d;
	being ca = beingListGetClosest(e->bl, animalGetBeing(e), BEING_ANIMAL, &d);
	if((ca == 0) || (d > 1)){return;}
	e->rot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
	e->gvel.x = dir.x;
	e->gvel.z = dir.z;
	if(!(e->flags & ANIMAL_FALLING)){
		e->vel.y  = 0.03f;
	}
	stateChange[ANIMAL_S_FLEE] += 256;
}

static void werebunnyStateChange(animal *e,int stateChange[16]){
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

void werebunnySearchPrey(animal *e,int stateChange[16]){
	(void)stateChange;
	if((e->state != ANIMAL_S_LOITER) && (e->state != ANIMAL_S_PLAYING) && (e->state != ANIMAL_S_FOOD_SEARCH)){return;}
	if(e->hunger > 70) {return;}

	float d;
	being b  = beingListGetClosest(beingListGet(e->pos.x,e->pos.y,e->pos.z), animalGetBeing(e), BEING_ANIMAL, &d);
	if(d > 32.f){return;}
	animal *oa = animalGetByBeing(b);
	if((oa == NULL) || (oa->type != 1)){return;}

	e->state  = ANIMAL_S_HUNT;
	e->target = b;
}

void animalThinkWerebunny(animal *e){
	static int stateChange[16];
	for(uint i=0;i<16;i++){
		stateChange[i] = rngValA(63);
	}
	stateChange[ANIMAL_S_FIGHT] = 0;
	stateChange[ANIMAL_S_FLEE]  = 0;

	e->stateTicks++;
	animalCheckSuffocation(e);
	werebunnyFightOrFlight   (e,stateChange);
	werebunnyAgeing          (e,stateChange);
	werebunnySleepyness      (e,stateChange);
	werebunnyHunger          (e,stateChange);
	werebunnyPoop            (e,stateChange);
	werebunnySocialDistancing(e,stateChange);
	werebunnySearchPrey      (e,stateChange);

	int tcat = gtimeGetTimeCat();
	if((tcat == TIME_NIGHT) || (tcat == TIME_EVENING)){
		stateChange[ANIMAL_S_SLEEP] += rngValA(255);
		stateChange[ANIMAL_S_SLEEP] *= 16;
	}else{
		stateChange[ANIMAL_S_SLEEP] = MAX(1,stateChange[ANIMAL_S_SLEEP]);
		stateChange[ANIMAL_S_SLEEP] /= 16;
	}

	switch(e->state){
	default:
	case ANIMAL_S_LOITER:
		werebunnySLoiter(e,stateChange);
		break;
	case ANIMAL_S_FLEE:
		werebunnySFlee(e,stateChange);
		break;
	case ANIMAL_S_HEAT:
		break;
	case ANIMAL_S_SLEEP:
		werebunnySSleep(e,stateChange);
		break;
	case ANIMAL_S_PLAYING:
		werebunnySPlayful(e,stateChange);
		break;
	case ANIMAL_S_FOOD_SEARCH:
		werebunnySFoodSearch(e,stateChange);
		break;
	case ANIMAL_S_EAT:
		werebunnySEat(e,stateChange);
		break;
	case ANIMAL_S_FIGHT:
		werebunnySFight(e,stateChange);
		break;
	case ANIMAL_S_HUNT:
		werebunnySHunt(e,stateChange);
		break;
	}
	werebunnyStateChange(e,stateChange);
}

void animalRBurnWerebunny(animal *e){
	e->state = ANIMAL_S_FLEE;
	if(!(e->flags & ANIMAL_FALLING)){
		e->vel.y = 0.04f;
	}
}


void animalRDieWerebunny(animal *e){
	item mdrop = itemNew(I_Meat,rngValMM(4,6));
	item fdrop = itemNew(I_Fur, rngValMM(3,4));
	itemDropNewP(e->pos,&mdrop,-1);
	itemDropNewP(e->pos,&fdrop,-1);
}
