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
#include "entity.h"

#include "../game/being.h"
#include "../game/blockType.h"
#include "../misc/effects.h"
#include "../misc/profiling.h"
#include "../world/world.h"
#include "../nujel/entity.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

entity  entityList[1<<14];
uint    entityCount = 0;
uint    entityMax = 0;
entity *entityFirstFree = NULL;
uint    entityUpdateCount = 0;

u32     entityGeneration = 0;

void entityReset(entity *e){
	memset(e,0,sizeof(entity));
}

entity *entityNew(vec pos, vec rot, float weight){
	if(isClient){
		fprintf(stderr, "Can't use entityNew on clients!\n");
		exit(3);
		return NULL;
	}
	entity *e = NULL;
	if(entityFirstFree == NULL){
		e = &entityList[entityMax++];
	}else{
		e = entityFirstFree;
		entityFirstFree = e->nextFree;
		if(entityFirstFree == e){
			entityFirstFree = NULL;
		}
	}
	entityReset(e);

	e->generation = ++entityGeneration;
	e->pos = pos;
	e->rot = rot;
	e->weight = weight;
	e->handler = NULL;
	entityCount++;

	return e;
}

void entityFree(entity *e){
	if(e == NULL){return;}
	if(isClient){
		fprintf(stderr, "Can't use entityNew on clients!\n");
		exit(3);
		return;
	}
	entityCount--;
	e->handler = NULL;
	e->nextFree = entityFirstFree;
	entityFirstFree = e;
	if(e->nextFree == NULL){
		e->nextFree = e;
	}
}

u32 entityCollision(const vec c){
	u32 col = 0;

	if(checkCollision(c.x-0.3f,c.y     ,c.z     )){col |= 0x100;}
	if(checkCollision(c.x+0.3f,c.y     ,c.z     )){col |= 0x200;}
	if(checkCollision(c.x     ,c.y     ,c.z-0.3f)){col |= 0x400;}
	if(checkCollision(c.x     ,c.y     ,c.z+0.3f)){col |= 0x800;}
	if(checkCollision(c.x     ,c.y+0.5f,c.z     )){col |=  0xF0;}
	if(checkCollision(c.x     ,c.y-0.5f,c.z     )){col |=   0xF;}

	return col;
}

u8 entityCollisionBlock(const vec c, vec *retPos){
	blockId b;
	if((b = worldGetB(c.x-0.3f,c.y     ,c.z     ))){*retPos = vecNew(c.x-0.3f,c.y     ,c.z     ); return b;}
	if((b = worldGetB(c.x+0.3f,c.y     ,c.z     ))){*retPos = vecNew(c.x+0.3f,c.y     ,c.z     ); return b;}
	if((b = worldGetB(c.x     ,c.y     ,c.z-0.3f))){*retPos = vecNew(c.x     ,c.y     ,c.z-0.3f); return b;}
	if((b = worldGetB(c.x     ,c.y     ,c.z+0.3f))){*retPos = vecNew(c.x     ,c.y     ,c.z+0.3f); return b;}
	if((b = worldGetB(c.x     ,c.y+0.5f,c.z     ))){*retPos = vecNew(c.x     ,c.y+0.5f,c.z     ); return b;}
	if((b = worldGetB(c.x     ,c.y-0.5f,c.z     ))){*retPos = vecNew(c.x     ,c.y-0.5f,c.z     ); return b;}
	return 0;
}

void entityUpdateCurChungus(entity *e){
	const int cx = (int)e->pos.x >> 8;
	const int cy = (int)e->pos.y >> 8;
	const int cz = (int)e->pos.z >> 8;
	e->curChungus = worldTryChungus(cx,cy,cz);
}

static float entityBlockRepulsion(entity *c, float *vel){
	return blockRepulsion(c->pos,vel,c->weight,entityCollisionBlock);
}

int entityUpdate(entity *e){
	int ret = 0;
	u32 col;
	if((e->flags & ENTITY_SLOW_UPDATE) && (entityUpdateCount & 0x3F)){return 0;}
	e->flags &= ~ENTITY_SLOW_UPDATE;
	if(!worldShouldBeLoaded(e->pos)){return -1;}
	if(!vecInWorld(e->pos))         {return  1;}
	e->pos = vecAdd(e->pos,e->vel);
	if(e->flags & ENTITY_NOCLIP){
		e->flags &= ~ENTITY_COLLIDE;
		if(entityCollision(e->pos)){ e->flags |= ENTITY_COLLIDE; }
		entityUpdateCurChungus(e);
		return 0;
	}
	e->vel.y -= 0.0005f;
	// ToDo: implement terminal velocity in a better way
	if(e->vel.y < -1.0f){e->vel.y += 0.0007f;}
	if(e->vel.y >  1.0f){e->vel.y -= 0.0007f;}
	if(e->vel.x < -1.0f){e->vel.x += 0.0007f;}
	if(e->vel.x >  1.0f){e->vel.x -= 0.0007f;}
	if(e->vel.z < -1.0f){e->vel.z += 0.0007f;}
	if(e->vel.z >  1.0f){e->vel.z -= 0.0007f;}

	if(e->yoff > 0.01f){
		e->yoff -= 0.01f;
	}else if(e->yoff < -0.01f){
		e->yoff += 0.01f;
	}
	bool wasColliding = e->flags & ENTITY_COLLIDE;

	e->flags |=  ENTITY_FALLING;
	e->flags &= ~ENTITY_COLLIDE;
	col = entityCollision(e->pos);
	if(col){ e->flags |= ENTITY_COLLIDE; }
	e->flags |= ENTITY_UPDATED;
	if(e->flags & ENTITY_NOREPULSION){
		entityUpdateCurChungus(e);
		return 0;
	}
	if((col&0x110) && (e->vel.x < 0.f)){
		if(e->vel.x < -0.1f){
			ret += entityBlockRepulsion(e,&e->vel.x);
		} else {
			e->vel.x *= -0.97f;
		}
		e->pos.x = MAX(e->pos.x,floor(e->pos.x)+0.3f);
	}
	if((col&0x220) && (e->vel.x > 0.f)){
		if(e->vel.x >  0.1f){
			ret += entityBlockRepulsion(e,&e->vel.x);
		}else{
			e->vel.x *= -0.97f;
		}
		e->pos.x = MIN(e->pos.x,floorf(e->pos.x)+0.7f);
	}
	if((col&0x880) && (e->vel.z > 0.f)){
		if(e->vel.z >  0.1f){
			ret += entityBlockRepulsion(e,&e->vel.z);
		} else {
			e->vel.z *= -0.97f;
		}
		e->pos.z = MIN(e->pos.z,floorf(e->pos.z)+0.7f);
	}
	if((col&0x440) && (e->vel.z < 0.f)){
		if(e->vel.z < -0.1f){
			ret += entityBlockRepulsion(e,&e->vel.z);
		}else{
			e->vel.z *= -0.97f;
		}
		e->pos.z = MAX(e->pos.z,floorf(e->pos.z)+0.3f);
	}
	if((col&0x0F0) && (e->vel.y > 0.f)){
		e->pos.y = MIN(e->pos.y,floorf(e->pos.y)+0.3f); // Still needed?
		if(e->vel.y >  0.1f){
			ret += entityBlockRepulsion(e,&e->vel.y);
		}else{
			e->vel.y *= 0.97f;
		}
	}
	if((col&0x00F) && (e->vel.y < 0.f)){
		e->flags &= ~ENTITY_FALLING;
		if(e->vel.y < -0.15f){
			e->yoff = -0.8f;
			ret += entityBlockRepulsion(e,&e->vel.y);
		}else{
			if(e->vel.y < -0.07f){
				e->yoff += -0.4f;
			}else if(e->vel.y < -0.04f){
				e->yoff += -0.2f;
			}
			e->vel = vecMul(e->vel,vecNew(0.93f,0,0.93f));
			e->pos.y = floorf(e->pos.y)+.5f;
			if(vecMag(e->vel) < 0.001f){
				e->flags |= ENTITY_SLOW_UPDATE;
			}
		}
	}
	col = entityCollision(e->pos);

	entityUpdateCurChungus(e);
	e->flags &= ~ENTITY_UPDATED;

	if(!wasColliding && (e->flags & ENTITY_COLLIDE)){
		const int SP = lRootsGet();
		lVal *msg = RVP(lCons(RVP(lValSym(":collision")),NULL));
		lEntityEvent(e, msg);
		lRootsRet(SP);
	}
	return ret;
}

float entityDistance(const entity *e, const character *c){
	return vecMag(vecSub(e->pos,c->pos));
}

uint lineOfSightBlockCount(const vec a, const vec b, uint maxB){
	float lastDist = 999999.f;
	uint ret = 0;
	vec p = a;
	u64 lastB = -1;
	vec dist = vecSub(b,p);
	if(vecMag(dist) > 128.f){return maxB+1;}

	for(uint max=1<<10;max;--max){
		dist = vecSub(b,p);
		const float curDist = vecMag(dist);
		if(curDist > lastDist){break;}
		lastDist = curDist;
		const vec v = vecMulS(vecNorm(dist),0.5f);
		p = vecAdd(p,v);
		const u64 newB = vecToPacked(p);
		if(newB != lastB && worldGetB(p.x,p.y,p.z)){
			lastB = newB;
			if(++ret > maxB){return ret;}
		}

	}
	return ret;
}

void entityUpdateAll(){
	PROFILE_START();

	for(uint i=0;i<entityMax;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(!(entityList[i].flags & ENTITY_UPDATED)){
			entityUpdate(&entityList[i]);
		}else{
			entityList[i].flags &= ~ENTITY_UPDATED;
		}
	}
	entityUpdateCount++;

	PROFILE_STOP();
}

float blockRepulsion(const vec pos, float *vel, float weight, u8 (*colFunc)(const vec,vec *)){
	int ret = 0;
	vec blockPos;
	const u8 b = colFunc(pos,&blockPos);
	if(b == 0){return 0;}

	float strength = *vel;
	const int blockDamage = fabsf(strength) * weight * 50.f;
	const int blockHealth = blockTypeGetHealth(b);
	if(blockDamage > blockHealth){
		worldBreak(blockPos.x,blockPos.y,blockPos.z);
		strength = strength * ((float)blockHealth / (float)blockDamage);
		fxBlockBreak(vecFloor(blockPos),b,0);
	}

	if(fabsf(strength) > 0.1f){ret += (int)(fabsf(strength)*512.f);}
	*vel += strength* -0.7f;

	return ret;
}

i64 entityID(const entity *e){
	if(e < entityList){return 0;}
	if(e >= &entityList[entityMax]){return 0;}
	const i32 id = (e - entityList);
	i64 ret = (i64)e->generation | ((i64)id << 32);
	return ret;
}

entity *entityGetByID(i64 raw){
	const i64 id = (raw >> 32) & 0xFFFFFFFF;
	if(id >= entityMax){
		return NULL;
	}
	entity *ent = &entityList[id];
	if(ent->nextFree){return NULL;}
	const i64 generation = raw & 0xFFFF;
	return generation == ent->generation ? ent : NULL;
}

entity *entityGetByBeing(being b){
	const uint i = beingID(b);
	if(beingType(b) != bkEntity){ return NULL; }
	if(i >= entityMax)             { return NULL; }
	return &entityList[i];
}

being entityGetBeing(const entity *e){
	if(e == NULL){return 0;}
	return beingEntity(e - entityList);
}
