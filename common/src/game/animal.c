#include "animal.h"

#include "../mods/api_v1.h"

#include <string.h>
#include <math.h>

animal  animalList[1<<10];
uint    animalCount = 0;
animal *animalFirstFree = NULL;

void animalReset(animal *e){
	memset(e,0,sizeof(animal));
}

animal *animalNew(const vec pos , int type, int gender){
	animal *e = NULL;
	if(animalCount >= ((sizeof(animalList) / sizeof(animal)-1))){return NULL;}
	e = &animalList[animalCount++];
	animalReset(e);

	e->pos       = pos;
	e->rot       = vecZero();
	e->grot      = vecZero();
	e->gvel      = vecZero();

	e->age       = 21;
	e->hunger    = 64;
	e->sleepy    = 64;
	e->pregnancy = -1;

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

	return e;
}

void animalDel(uint i){
	if(i >= animalCount) {return;}
	animalList[i] = animalList[--animalCount];
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
	const vec cFDir = vecAdd(e->pos,vecMulS(vecNorm(vecMul(e->gvel,vecNew(1,0,1))),0.5f));
	const u8 cb     = worldGetB(cFDir.x, cFDir.y, cFDir.z);
	if((cb != 0)){
		if(!(e->flags & ANIMAL_FALLING)){
			e->vel.y = 0.04f;
		}
	}else if(cb == 0){
		const vec cBDir = vecAdd(e->pos,vecMulS(vecNorm(vecMul(e->gvel,vecNew(1,0,1))),-0.5f));
		for(int cy = 1; cy < 8; cy++){
			if(worldGetB(cFDir.x,cFDir.y-cy,cFDir.z)   != 0){return;}
		}
		for(int cy = 1; cy < 5; cy++){
			if(cy == 4){return;}
			if(worldGetB(cBDir.x,cBDir.y-cy,cBDir.z)   != 0){break;}
		}

		vec tmp = e->gvel;
		tmp.y   = 0.03f;
		e->vel  = vecMul(tmp,vecNew(-4,1,-4));
	}
}

int animalUpdate(animal *e){
	int ret=0;
	u32 col;
	if(e->type == 0){return 0;}
	e->pos = vecAdd(e->pos,e->vel);
	e->breathing += 5;
	animalCheckForHillOrCliff(e);

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

	if(fabsf(e->vel.x - e->gvel.x) > 0.001f){
		if(e->vel.x > e->gvel.x){
			e->vel.x -= 0.005f;
		}else{
			e->vel.x += 0.005f;
		}
	}
	if(fabsf(e->vel.z - e->gvel.z) > 0.001f){
		if(e->vel.z > e->gvel.z){
			e->vel.z -= 0.005f;
		}else{
			e->vel.z += 0.005f;
		}
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
