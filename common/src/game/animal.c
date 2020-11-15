#include "animal.h"

#include "../animals/bunny.h"
#include "../animals/guardian.h"
#include "../game/character.h"
#include "../game/rope.h"
#include "../mods/api_v1.h"
#include "../network/messages.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

animal  animalList[1<<10];
uint    animalCount = 0;
uint    animalUsedCount = 0;
uint    animalFirstFree = 0xFFFF;

void animalReset(animal *e){
	memset(e,0,sizeof(animal));
}

animal *animalNew(const vec pos , int type, int gender){
	animal *e = NULL;
	if(animalFirstFree < (1<<10)){
		e = &animalList[animalFirstFree];
		animalFirstFree = e->nextFree;
	}else{
		if(animalCount >= ((sizeof(animalList) / sizeof(animal)-1))){return NULL;}
		e = &animalList[animalCount++];
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

	if(rngValM(2) == 0){
		e->flags |= ANIMAL_BELLYSLEEP;
	}
	if(rngValM(2) == 0){
		e->flags |= ANIMAL_AGGRESIVE;
	}

	if(gender < 0){
		if(rngValM(2) == 0){
			e->flags |= ANIMAL_MALE;
		}
	}else if(gender == 1){
		e->flags |= ANIMAL_MALE;
	}

	if(type == 2){
		e->flags |= ANIMAL_NO_NEEDS;
	}

	return e;
}

void animalDel(uint i){
	if(i >= animalCount) {return;}
	animalList[i].type     = 0;
	animalList[i].nextFree = animalFirstFree;
	animalFirstFree        = i;
}

u32 animalCollision(const vec c){
	u32 col = 0;

	if(checkCollision(c.x-0.3f,c.y     ,c.z     )){col |= 0x100;}
	if(checkCollision(c.x+0.3f,c.y     ,c.z     )){col |= 0x200;}
	if(checkCollision(c.x     ,c.y     ,c.z-0.3f)){col |= 0x400;}
	if(checkCollision(c.x     ,c.y     ,c.z+0.3f)){col |= 0x800;}
	if(checkCollision(c.x     ,c.y+0.5f,c.z     )){col |= 0x0F0;}
	if(checkCollision(c.x     ,c.y-1.0f,c.z     )){col |= 0x00F;}

	return col;
}

void animalUpdateCurChungus(animal *e){
	const int cx = (int)e->pos.x >> 8;
	const int cy = (int)e->pos.y >> 8;
	const int cz = (int)e->pos.z >> 8;
	e->curChungus = worldTryChungus(cx,cy,cz);
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

int animalUpdate(animal *e){
	int ret=0;
	u32 col;
	if(e->type == 0)       {return 0;}
	e->pos = vecAdd(e->pos,e->vel);
	if(!vecInWorld(e->pos)){return 1;}
	e->breathing += 5;
	animalCheckForHillOrCliff(e);

	if(vecSum(vecAbs(e->gvel)) > 2.f){
		if(isClient){
			fprintf(stderr,"Client: ");
		}else{
			fprintf(stderr,"Server: ");
		}
		fprintf(stderr,"Animal[%u] wants to go too fast\n Vel X:%f Y:%f Z:%f\n GVL X:%f Y:%f Z:%f\n\n",(int)(e-animalList),e->vel.x,e->vel.y,e->vel.z,e->gvel.x,e->gvel.y,e->gvel.z);
	}
	if(vecSum(vecAbs(e->vel)) > 2.f){
		if(isClient){
			fprintf(stderr,"Client: ");
		}else{
			fprintf(stderr,"Server: ");
		}
		fprintf(stderr,"Animal[%u] going too fast\n Vel X:%f Y:%f Z:%f\n GVL X:%f Y:%f Z:%f\n\n",(int)(e-animalList),e->vel.x,e->vel.y,e->vel.z,e->gvel.x,e->gvel.y,e->gvel.z);
	}

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
	// ToDo: implement terminal veolocity in a better way
	if(e->vel.y < -1.0f){e->vel.y+=0.005f;}
	if(e->vel.y >  1.0f){e->vel.y-=0.005f;}

	e->flags |=  ANIMAL_FALLING;
	e->flags &= ~ANIMAL_COLLIDE;
	col = animalCollision(e->pos);
	if(col){ e->flags |= ANIMAL_COLLIDE; }

	if((col&0x110) && (e->vel.x < 0.f)){
		if(e->vel.x < -0.05f){ ret += (int)(fabsf(e->vel.x)*24.f); }
		e->pos.x = MAX(e->pos.x,floor(e->pos.x)+0.3f);
		e->vel.x = e->vel.x*-0.3f;
	}
	if((col&0x220) && (e->vel.x > 0.f)){
		if(e->vel.x >  0.05f){ ret += (int)(fabsf(e->vel.x)*24.f); }
		e->pos.x = MIN(e->pos.x,floorf(e->pos.x)+0.7f);
		e->vel.x = e->vel.x*-0.3f;
	}
	if((col&0x880) && (e->vel.z > 0.f)){
		if(e->vel.z >  0.05f){ ret += (int)(fabsf(e->vel.z)*24.f); }
		e->pos.z = MIN(e->pos.z,floorf(e->pos.z)+0.7f);
		e->vel.z = e->vel.z*-0.3f;
	}
	if((col&0x440) && (e->vel.z < 0.f)){
		if(e->vel.z < -0.05f){ ret += (int)(fabsf(e->vel.z)*24.f); }
		e->pos.z = MAX(e->pos.z,floorf(e->pos.z)+0.3f);
		e->vel.z = e->vel.z*-0.3f;
	}
	if((col&0x0F0) && (e->vel.y > 0.f)){
		if(e->vel.y >  0.05f){ ret += (int)(fabsf(e->vel.y)*24.f); }
		e->pos.y = MIN(e->pos.y,floorf(e->pos.y)+0.5f);
		e->vel.y = e->vel.y*-0.3f;
	}
	if((col&0x00F) && (e->vel.y < 0.f)){
		e->flags &= ~ANIMAL_FALLING;
		if(e->vel.y < -0.05f){
			ret += (int)(fabsf(e->vel.y)*24.f);
		}
		e->vel = vecMul(e->vel,vecNew(0.97f,0,0.97f));
	}

	animalUpdateCurChungus(e);
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
	}
}

int animalGetMaxHealth (const animal *e){
	switch(e->type){
	default:
	case 1:
		return  4;
	case 2:
		return 12;
	}
}

animal *animalGetByBeing(being b){
	const uint i = beingID(b);
	if(beingType(b) != BEING_ANIMAL){ return NULL; }
	if(i >= animalCount)            { return NULL; }
	return &animalList[i];
}

being animalGetBeing(const animal *c){
	if(c == NULL){return 0;}
	return beingAnimal(c - &animalList[0]);
}

animal *animalClosest(const vec pos, float maxDistance){
	for(uint i=0;i<animalCount;i++){
		if(animalList[i].type == 0){continue;}
		const float d = vecMag(vecSub(pos,animalList[i].pos));
		if(d > maxDistance){continue;}
		return &animalList[i];
	}
	return NULL;
}

void animalUpdateAll(){
	for(int i=animalCount-1;i>=0;i--){
		if(animalList[i].type == 0){continue;}
		int dmg = animalUpdate(&animalList[i]);
		animalList[i].health -= dmg;
		if(isClient){continue;}
		if((animalList[i].pos.y  < 0.f) ||
		   (animalList[i].health <= 0) ||
		   (animalList[i].hunger <= 0) ||
		   (animalList[i].sleepy <= 0)) {
			animalRDie(&animalList[i]);
			animalDel(i);
			continue;
		}
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
	for(uint i=0;i<animalCount;i++){
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

void animalRDie(animal *e){
	if(e->pos.y > 0.f){
		switch(e->type){
		default:
			break;
		case 1:
			animalRDieBunny(e);
			break;
		case 2:
			animalRDieGuardian(e);
			break;
		}
	}
	if(!isClient){
		msgAnimalDied(-1,e);
		ropeDelBeing(animalGetBeing(e));
	}
}


void animalRHit(animal *e){
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

void animalThink(animal *e){
	switch(e->type){
	default:
		return;
	case 1:
		animalThinkBunny(e);
		break;
	case 2:
		animalThinkGuardian(e);
		break;
	}
}

void animalThinkAll(){
	for(int i=animalCount-1;i>=0;--i){
		animalThink(&animalList[i]);
	}
}

void animalNeedsAll(){
	static uint calls = 0;
	for(int i=animalCount-1;i>=0;--i){
		animal *e = &animalList[i];
		if(e->flags & ANIMAL_NO_NEEDS){continue;}
		e->hunger--;
		e->sleepy--;
		if((calls & 0x3F) == 0){e->age++;}
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
	rp->v.u16[5] = animalCount;

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

	packetQueue(rp,30,16*4,c);
}

int animalHitCheck(const vec pos, float mdd, int dmg, int cause, u16 iteration, being source){
	int hits = 0;
	for(uint i=0;i<animalCount;i++){
		if(animalList[i].type == 0)        {continue;}
		if(animalList[i].temp == iteration){continue;}
		if(beingAnimal(i) == source)       {continue;}
		const vec d = vecSub(pos,animalList[i].pos);
		if(vecDot(d,d) < mdd){
			if(isClient){msgBeingDamage(0,dmg,cause,beingAnimal(i),0,pos);}
			animalList[i].temp = iteration;
			hits++;
		}
	}
	return hits;
}
