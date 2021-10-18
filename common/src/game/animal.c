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

#include "animal.h"

#include "../animals/bunny.h"
#include "../animals/guardian.h"
#include "../animals/werebunny.h"
#include "../game/being.h"
#include "../game/blockType.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/fire.h"
#include "../game/rope.h"
#include "../game/time.h"
#include "../misc/profiling.h"
#include "../network/messages.h"
#include "../world/world.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#define ANIMAL_FALL_DMG 12.f
#define ANIMAL_MAX (1<<12)

animal  animalList[ANIMAL_MAX];
uint    animalListMax   = 0;
uint    animalCount     = 0;
uint    animalFirstFree = 0xFFFFFFFF;

void animalReset(animal *e){
	memset(e,0,sizeof(animal));
}

animal *animalNew(const vec pos , int type, int gender){
	animal *e = NULL;
	if(type == 0){return NULL;}
	if(animalFirstFree < ANIMAL_MAX){
		e = &animalList[animalFirstFree];
		animalFirstFree = e->nextFree;
	}else{
		if(animalListMax >= countof(animalList)){
			e = &animalList[rngValA(ANIMAL_MAX-1)];
			animalDel(e-animalList);
			return animalNew(pos,type,gender);
		}
		e = &animalList[animalListMax++];
	}
	animalReset(e);

	e->pos       = pos;
	e->rot       = vecZero();
	e->grot      = vecZero();
	e->gvel      = vecZero();

	e->age       = 21;
	e->hunger    = 64;
	e->sleepy    = 64;
	e->pregnancy = -1;
	e->nextFree  = 0xFFFF;

	e->type      = type;
	e->health    = animalGetMaxHealth(e);

	if(rngValM(2) == 0){e->flags |= ANIMAL_BELLYSLEEP;}
	if(rngValM(2) == 0){e->flags |= ANIMAL_AGGRESIVE;}

	if(gender < 0){
		if(rngValM(2) == 0){e->flags |= ANIMAL_MALE;}
	}else if(gender == 1){
		e->flags |= ANIMAL_MALE;
	}

	if(type == animalGuardian){e->flags |= ANIMAL_NO_NEEDS;}
	animalCount++;

	return e;
}

void animalDel(uint i){
	if(i >= animalListMax){
		printf("AnimalDel: %u > %u\n",i,animalListMax);
		return;
	}
	beingListDel(animalList[i].bl,beingAnimal(i));
	if(animalList[i].type == 0){
		printf("AnimalDel: type==0!!!\n");
		return;
	}
	animalList[i].pos      = vecNOne();
	animalList[i].bl       = NULL;
	animalList[i].type     = 0;
	animalList[i].nextFree = animalFirstFree;
	animalFirstFree        = i;
	animalCount--;
}

void animalDelChungus(const chungus *c){
	if(c == NULL){return;}
	const vec cp = chungusGetPos(c);
	for(uint i=0;i<animalListMax;i++){
		if(animalList[i].type == 0)  {continue;}
		const vec *p = &animalList[i].pos;
		if(((int)p->x >> 8) != cp.x){continue;}
		if(((int)p->y >> 8) != cp.y){continue;}
		if(((int)p->z >> 8) != cp.z){continue;}
		animalDel(i);
	}
}

u32 animalCollision(const vec c){
	u32 col = 0;
	float size = 1.f;

	if(checkCollision(c.x-0.3f*size,c.y     ,c.z          )){col |= 0x100;}
	if(checkCollision(c.x+0.3f*size,c.y     ,c.z          )){col |= 0x200;}
	if(checkCollision(c.x     ,c.y          ,c.z-0.3f*size)){col |= 0x400;}
	if(checkCollision(c.x     ,c.y          ,c.z+0.3f*size)){col |= 0x800;}
	if(checkCollision(c.x     ,c.y+0.5f*size,c.z          )){col |= 0x0F0;}
	if(checkCollision(c.x     ,c.y-1.0f*size,c.z          )){col |= 0x00F;}

	return col;
}

u8 animalCollisionBlock(const vec c, vec *retPos){
	u8 b;
	if((b = worldGetB(c.x-0.3f,c.y     ,c.z     ))){*retPos = vecNew(c.x-0.3f,c.y,c.z); return b;}
	if((b = worldGetB(c.x+0.3f,c.y     ,c.z     ))){*retPos = vecNew(c.x+0.3f,c.y,c.z); return b;}
	if((b = worldGetB(c.x     ,c.y     ,c.z-0.3f))){*retPos = vecNew(c.x,c.y,c.z-0.3f); return b;}
	if((b = worldGetB(c.x     ,c.y     ,c.z+0.3f))){*retPos = vecNew(c.x,c.y,c.z+0.3f); return b;}
	if((b = worldGetB(c.x     ,c.y+0.5f,c.z     ))){*retPos = vecNew(c.x,c.y+0.5f,c.z); return b;}
	if((b = worldGetB(c.x     ,c.y-1.0f,c.z     ))){*retPos = vecNew(c.x,c.y-1.0f,c.z); return b;}
	return 0;
}

static float animalBlockRepulsion(animal *c, float *vel){
	return blockRepulsion(c->pos,vel,animalGetWeight(c),animalCollisionBlock);
}

void animalCheckForHillOrCliff(animal *e){
	const vec cFDir = vecAdd(e->pos,vecMulS(vecNorm(vecMul(e->vel,vecNew(1,0,1))),0.5f));
	const u8 cb     = worldGetB(cFDir.x, cFDir.y, cFDir.z);
	if((cb != 0)){
		if(!(e->flags & ANIMAL_FALLING)){
			e->vel.y = 0.04f;
		}
	}else if(cb == 0){
		const vec cBDir = vecAdd(e->pos,vecMulS(vecNorm(vecMul(e->vel,vecNew(1,0,1))),-0.5f));
		for(int cy = 1; cy < 8; cy++){
			if(worldGetB(cFDir.x,cFDir.y-cy,cFDir.z)   != 0){return;}
		}
		for(int cy = 1; cy < 5; cy++){
			if(cy == 4){return;}
			if(worldGetB(cBDir.x,cBDir.y-cy,cBDir.z)   != 0){break;}
		}

		vec tmp = e->vel;
		if(!(e->flags & ANIMAL_FALLING)){tmp.y   = 0.03f;}
		e->gvel = e->vel = vecMul(tmp,vecNew(-0.97f,1,-0.97f));
	}
}

void animalChaseTarget(animal *e){
	if(e->target == 0){return;}
	const vec tpos   = beingGetPos(e->target);
	const float dist = vecMag(vecSub(tpos,e->pos));
	if(dist > 256.f){return;}

	vec caNorm = vecNorm(vecNew(tpos.x - e->pos.x,0.f, tpos.z - e->pos.z));
	vec caVel  = vecMulS(caNorm,0.03f);
	vec caRot  = vecVecToDeg(caNorm);

	e->gvel.y  = 0.f;
	if(e->state == ANIMAL_S_FLEE){
		e->gvel.x  = -caVel.x;
		e->gvel.z  = -caVel.z;
		e->rot.yaw = -caRot.yaw + 180;
	}else{
		e->gvel.x  =  caVel.x;
		e->gvel.z  =  caVel.z;
		e->rot.yaw = -caRot.yaw;
		if(dist < 1.5f){
			e->gvel.x  = 0;
			e->gvel.z  = 0;
		}
	}
	e->yoff = cosf(gameTicks * 0.1f) * 0.1f;
}

int animalUpdate(animal *e){
	int ret=0;
	u32 col;
	if(e->type == 0){return 0;}
	e->pos = vecAdd(e->pos,e->vel);
	if(!worldShouldBeLoaded(e->pos)){return -1;}
	if(!vecInWorld(e->pos)){return 1;}
	e->breathing += 5;
	animalCheckForHillOrCliff(e);
	animalChaseTarget(e);

	if(fabsf(e->rot.yaw - e->grot.yaw) > 0.3f){
		if(e->rot.yaw > e->grot.yaw){
			e->rot.yaw -= 0.2f;
		}else{
			e->rot.yaw += 0.2f;
		}
	}
	if(fabsf(e->rot.pitch - e->grot.pitch) > 0.3f){
		if(e->rot.pitch > e->grot.pitch){
			e->rot.pitch -= 0.2f;
		}else{
			e->rot.pitch += 0.2f;
		}
	}

	if(!(e->flags & ANIMAL_FALLING)){
		e->vel = vecMulS(vecAdd(e->gvel,vecMulS(e->vel,31.f)),1.f/32.f);
	}

	if(e->rot.yaw   > 360.f){e->rot.yaw   -= 360.f;}
	if(e->rot.pitch > 180.f){e->rot.pitch -= 360.f;}

	e->vel.y -= 0.0005f;
	// ToDo: implement terminal velocity in a better way
	if(e->vel.y < -1.0f){e->vel.y+=0.005f;}
	if(e->vel.y >  1.0f){e->vel.y-=0.005f;}

	e->flags |=  ANIMAL_FALLING;
	e->flags &= ~ANIMAL_COLLIDE;
	col = animalCollision(e->pos);
	if(col){ e->flags |= ANIMAL_COLLIDE; }

	if((col&0x110) && (e->vel.x < 0.f)){
		if(e->vel.x < -0.05f){ ret += animalBlockRepulsion(e,&e->pos.x); }
		e->pos.x = MAX(e->pos.x,floor(e->pos.x)+0.3f);
	}
	if((col&0x220) && (e->vel.x > 0.f)){
		if(e->vel.x >  0.05f){ ret += animalBlockRepulsion(e,&e->pos.x); }
		e->pos.x = MIN(e->pos.x,floorf(e->pos.x)+0.7f);
	}
	if((col&0x880) && (e->vel.z > 0.f)){
		if(e->vel.z >  0.05f){ ret += animalBlockRepulsion(e,&e->pos.z); }
		e->pos.z = MIN(e->pos.z,floorf(e->pos.z)+0.7f);
	}
	if((col&0x440) && (e->vel.z < 0.f)){
		if(e->vel.z < -0.05f){ ret += animalBlockRepulsion(e,&e->pos.z); }
		e->pos.z = MAX(e->pos.z,floorf(e->pos.z)+0.3f);
	}
	if((col&0x0F0) && (e->vel.y > 0.f)){
		if(e->vel.y >  0.05f){ ret += animalBlockRepulsion(e,&e->pos.y); }
		e->pos.y = MIN(e->pos.y,floorf(e->pos.y)+0.5f);
	}
	if((col&0x00F) && (e->vel.y < 0.f)){
		e->flags &= ~ANIMAL_FALLING;
		if(e->vel.y < -0.15f){
			ret += animalBlockRepulsion(e,&e->vel.y);
		}else if(e->vel.y > -0.04f){
			e->vel = vecMul(e->vel,vecNew(0.97f,0,0.97f));
		}
		if(fabsf(e->vel.y) < 0.001f){
			e->pos.y = floorf(e->pos.y)+1.0f;
		}
	}
	return ret;
}

float animalDistance(const animal *e,const character *c){
	return vecMag(vecSub(e->pos,c->pos));
}

const char *animalGetStateName(const animal *e){
	switch(e->state){
	default:
	case ANIMAL_S_LOITER:
		return "Chill";
	case ANIMAL_S_FLEE:
		return "Flee";
	case ANIMAL_S_HEAT:
		return "Heat";
	case ANIMAL_S_SLEEP:
		return "Sleep";
	case ANIMAL_S_PLAYING:
		return "Play";
	case ANIMAL_S_FOOD_SEARCH:
		return "Food Search";
	case ANIMAL_S_EAT:
		return "Eat";
	case ANIMAL_S_FIGHT:
		return "Fight";
	case ANIMAL_S_HUNT:
		return "Hunt";
	}
}

int animalGetMaxHealth(const animal *e){
	switch((animalType)e->type){
	case animalUnused:
		return  0;
	case animalBunny:
		return  6;
	case animalGuardian:
		return 12;
	case animalWerebunny:
		return 20;
	}
	return 0;
}

float animalGetWeight(const animal *e){
	switch((animalType)e->type){
	case animalUnused:
		return 2.f;
	case animalBunny:
		return 2.f;
	case animalGuardian:
		return 100.f;
	case animalWerebunny:
		return 6.f;
	}
	return 0;
}

animal *animalGetByBeing(being b){
	const uint i = beingID(b);
	if(beingType(b) != BEING_ANIMAL){ return NULL; }
	if(i >= animalListMax)          { return NULL; }
	return &animalList[i];
}

being animalGetBeing(const animal *c){
	if(c == NULL){return 0;}
	return beingAnimal(c - &animalList[0]);
}

animal *animalClosest(const vec pos, float maxDistance){
	for(uint i=0;i<animalListMax;i++){
		if(animalList[i].type == 0){continue;}
		const float d = vecMag(vecSub(pos,animalList[i].pos));
		if(d > maxDistance){continue;}
		return &animalList[i];
	}
	return NULL;
}

static void animalUpdateBL(){
	for(uint i=0;i<animalListMax;i++){
		if(animalList[i].type == 0){continue;}
		animal *a = &animalList[i];
		a->bl = beingListUpdate(a->bl,animalGetBeing(a));
	}
}

void animalUpdateAll(){
	PROFILE_START();

	for(int i=animalListMax-1;i>=0;i--){
		if(animalList[i].type == 0){continue;}
		int dmg = animalUpdate(&animalList[i]);
		animalList[i].health -= dmg;
		if(isClient){continue;}
		if((animalList[i].pos.y  <  0.f) ||
		   (animalList[i].health <= 0)   ||
		   (animalList[i].hunger <= 0)   ||
		   (animalList[i].sleepy <= 0)   ||
		   (dmg < 0)) {
			animalRDie(&animalList[i]);
			animalDel(i);
			continue;
		}
	}
	animalUpdateBL();

	PROFILE_STOP();
}

void animalCheckTarget(animal *e){
	if(e->target == 0){return;}
	if(!beingAlive(e->target)){
		e->target = 0;
	}
}

float animalClosestPlayer(const animal *e, character **cChar){
	*cChar = NULL;
	float ret = 4096.f;
	for(uint i=0;i<characterCount;++i){
		if(characterList[i].hp <= 0){continue;}
		const float d = vecMag(vecSub(characterList[i].pos,e->pos));
		if(d < ret){
			ret = d;
			*cChar = &characterList[i];
		}
	}
	return ret;
}

float animalClosestAnimal(const animal *e, animal **cAnim, int typeFilter, uint flagsMask, uint flagsCompare){
	*cAnim = NULL;
	float ret = 256.f*256.f;
	for(uint i=0;i<animalListMax;i++){
		if(animalList[i].type == 0)                                {continue;}
		if(e == &animalList[i])                                    {continue;}
		if((typeFilter >= 0) && (animalList[i].type != typeFilter)){continue;}
		if((animalList[i].flags & flagsMask) != flagsCompare)      {continue;}
		const vec dist = vecSub(animalList[i].pos,e->pos);
		const float d = vecDot(dist,dist);
		if(d < ret){
			ret = d;
			*cAnim = &animalList[i];
		}
	}
	return sqrtf(ret);
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

being animalFindFOFTarget(const animal *e){
	character *cChar;
	float dist = animalClosestPlayer(e,&cChar);
	if(dist < 32.f){
		return characterGetBeing(cChar);
	}
	return 0;
}

void animalRDie(animal *e){
	if(e->pos.y > 0.f){
		switch((animalType)e->type){
		case animalUnused:
			break;
		case animalBunny:
			animalRDieBunny(e);
			break;
		case animalGuardian:
			animalRDieGuardian(e);
			break;
		case animalWerebunny:
			animalRDieWerebunny(e);
			break;
		}
	}
	if(!isClient){
		msgAnimalDied(-1,e);
		ropeDelBeing(animalGetBeing(e));
	}
}

void animalRHit(animal *e, being culprit, u8 cause){
	(void)cause;

	if(fabsf(e->vel.y) < 0.001f){
		e->vel.y = 0.03f;
	}
	e->target = culprit;
	if(e->state == ANIMAL_S_FLEE || (e->flags & ANIMAL_AGGRESIVE)){
		e->state = ANIMAL_S_FIGHT;
		e->flags |= ANIMAL_AGGRESIVE;
	}else{
		e->state = ANIMAL_S_FLEE;
	}
}

void animalThink(animal *e){
	switch((animalType)e->type){
	case animalUnused:
		return;
	case animalBunny:
		animalThinkBunny(e);
		break;
	case animalGuardian:
		animalThinkGuardian(e);
		break;
	case animalWerebunny:
		animalThinkWerebunny(e);
		break;
	}
}

void animalThinkAll(){
	PROFILE_START();

	static uint calls = 0;
	for(uint i=(calls&0x1F);i<animalListMax;i+=0x20){
		animalThink(&animalList[i]);
	}
	calls++;

	PROFILE_STOP();
}

void animalNeedsAll(){
	static uint calls = 0;
	PROFILE_START();

	int sleepyi = 1;
	int tcat = gtimeGetTimeCat();
	if((tcat == TIME_NIGHT) || (tcat == TIME_EVENING)){sleepyi = 2;}

	for(uint i=(calls&0xFFF);i<animalListMax;i+=0x1000){
		animal *e = &animalList[i];
		if(e->flags & ANIMAL_NO_NEEDS){continue;}
		e->hunger--;
		e->sleepy-=sleepyi;
		if(e->pregnancy > 0){e->pregnancy--;}
		if((calls & 0xF000) == 0){e->age++;}
	}
	calls++;
	PROFILE_STOP();
}

/* TODO: Add proper reaction to fire (runing away?) */
void animalRBurn(animal *e){
	switch((animalType)e->type){
	case animalUnused:
		return;
	case animalBunny:
		animalRBurnBunny(e);
		break;
	case animalGuardian:
		animalRBurnGuardian(e);
		break;
	case animalWerebunny:
		animalRBurnWerebunny(e);
		break;
	}
}

void animalCheckBurnAll(){
	static uint calls = 0;
	for(uint i=(calls&0x7F);i<animalListMax;i+=0x80){
		animal *a = &animalList[i];
		fire *f = fireGetAtPos(a->pos.x,a->pos.y,a->pos.z);
		if(f == NULL)       {continue;}
		if(f->strength < 64){continue;}
		a->health--;
		animalRBurn(a);
	}
	calls++;
}

void animalSync(u8 c, u16 i){
	packet *rp = &packetBuffer;
	const animal *e = &animalList[i];

	rp->v.u8[ 0] = e->type;
	rp->v.u8[ 1] = e->flags;
	rp->v.u8[ 2] = e->state;
	rp->v.i8[ 3] = e->age;

	rp->v.i8[ 4] = e->health;
	rp->v.i8[ 5] = e->hunger;
	rp->v.i8[ 6] = e->pregnancy;
	rp->v.i8[ 7] = e->sleepy;

	rp->v.u16[4] = i;
	rp->v.u16[5] = animalListMax;

	rp->v.f[ 3]  = e->pos.x;
	rp->v.f[ 4]  = e->pos.y;
	rp->v.f[ 5]  = e->pos.z;

	rp->v.f[ 6]  = e->vel.x;
	rp->v.f[ 7]  = e->vel.y;
	rp->v.f[ 8]  = e->vel.z;

	rp->v.f[ 9]  = e->gvel.x;
	rp->v.f[10]  = e->gvel.y;
	rp->v.f[11]  = e->gvel.z;

	rp->v.f[12]  = e->rot.yaw;
	rp->v.f[13]  = e->rot.pitch;

	rp->v.f[14]  = e->grot.yaw;
	rp->v.f[15]  = e->grot.pitch;

	rp->v.u32[16] = e->target;

	packetQueue(rp,msgtAnimalSync,17*4,c);
}

void animalEmptySync(u8 c){
	packet *rp = &packetBuffer;
	memset(rp->v.u8,0,17*4);
	packetQueue(rp,msgtAnimalSync,17*4,c);
}

void animalSyncInactive(u8 c, u16 i){
	packet *rp = &packetBuffer;

	rp->v.u8[ 0] = 0;

	rp->v.u16[4] = i;
	rp->v.u16[5] = animalListMax;

	packetQueue(rp,msgtAnimalSync,17*4,c);
}

int animalHitCheck(const vec pos, float mdd, int dmg, int cause, u16 iteration, being source){
	int hits = 0;
	beingList *bl = beingListGet(pos.x,pos.y,pos.z);
	if(bl == NULL){return 0;}
	for(beingListEntry *ble = bl->first;ble != NULL;ble = ble->next){
		for(uint i=0;i<countof(ble->v);i++){
			if(beingType(ble->v[i]) != BEING_ANIMAL){continue;}
			if(source == ble->v[i]){continue;}
			animal *a = &animalList[ble->v[i] & (ANIMAL_MAX-1)];
			if(a->temp == iteration){continue;}
			const vec d = vecSub(pos,a->pos);
			if(vecDot(d,d) < mdd){
				if(isClient){msgBeingDamage(0,dmg,cause,1.f,ble->v[i],0,pos);}
				a->temp = iteration;
				hits++;
			}
		}
	}
	return hits;
}

void animalDoDamage(animal *a,i16 hp, u8 cause, float knockbackMult, being culprit, const vec pos){
	(void)culprit;

	a->health -= hp;
	if(a->health <= 0){
		animalRDie(a);
		animalDel(a - animalList);
		return;
	}
	animalRHit(a,culprit,cause);
	vec dis = vecNorm(vecSub(a->pos,pos));
	float knockback = 0.03f;
	if(cause != 2){knockback = 0.01f;}
	a->vel = vecAdd(a->vel,vecMulS(dis,knockback * knockbackMult));
}

const char *animalGetName(const animal *e){
	switch((animalType)e->type){
	case animalUnused:
		return  "An unknown entity";
	case animalBunny:
		return  "A little Bunny";
	case animalGuardian:
		return "A Guardian";
	case animalWerebunny:
		return "A massive Werebunny";
	}
	return "Unknown";
}

void animalDeleteAll(){
	for(uint i=0;i<animalListMax;i++){
		if(animalList[i].type == 0){continue;}
		animalDel(i);
	}
}
