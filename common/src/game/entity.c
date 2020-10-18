#include "entity.h"

#include "../mods/api_v1.h"

#include <string.h>
#include <math.h>

void entityReset(entity *e){
	memset(e,0,sizeof(entity));
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

void entityUpdateCurChungus(entity *e){
	const int cx = (int)e->pos.x >> 8;
	const int cy = (int)e->pos.y >> 8;
	const int cz = (int)e->pos.z >> 8;
	e->curChungus = worldTryChungus(cx,cy,cz);
}

int entityUpdate(entity *e){
	int ret=0;
	u32 col;
	e->pos = vecAdd(e->pos,e->vel);
	if(e->flags & ENTITY_NOCLIP){
		e->flags &= ~ENTITY_COLLIDE;
		if(entityCollision(e->pos)){ e->flags |= ENTITY_COLLIDE; }
		entityUpdateCurChungus(e);
		return 0;
	}

	e->vel.y -= 0.0005f;
	// ToDo: implement terminal veolocity in a better way
	if(e->vel.y < -1.0f){e->vel.y+=0.005f;}
	if(e->vel.y >  1.0f){e->vel.y-=0.005f;}

	if(e->yoff > 0.01f){
		e->yoff -= 0.01f;
	}else if(e->yoff < -0.01f){
		e->yoff += 0.01f;
	}

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
		if(e->vel.x < -0.1f){ ret += (int)(fabsf(e->vel.x)*128.f); }
		e->pos.x = MAX(e->pos.x,floor(e->pos.x)+0.3f);
		e->vel.x = e->vel.x*-0.3f;
	}
	if((col&0x220) && (e->vel.x > 0.f)){
		if(e->vel.x >  0.1f){ ret += (int)(fabsf(e->vel.x)*128.f); }
		e->pos.x = MIN(e->pos.x,floorf(e->pos.x)+0.7f);
		e->vel.x = e->vel.x*-0.3f;
	}
	if((col&0x880) && (e->vel.z > 0.f)){
		if(e->vel.z >  0.1f){ ret += (int)(fabsf(e->vel.z)*128.f); }
		e->pos.z = MIN(e->pos.z,floorf(e->pos.z)+0.7f);
		e->vel.z = e->vel.z*-0.3f;
	}
	if((col&0x440) && (e->vel.z < 0.f)){
		if(e->vel.z < -0.1f){ ret += (int)(fabsf(e->vel.z)*128.f); }
		e->pos.z = MAX(e->pos.z,floorf(e->pos.z)+0.3f);
		e->vel.z = e->vel.z*-0.3f;
	}
	if((col&0x0F0) && (e->vel.y > 0.f)){
		if(e->vel.y >  0.1f){ ret += (int)(fabsf(e->vel.y)*128.f); }
		e->pos.y = MIN(e->pos.y,floorf(e->pos.y)+0.5f);
		e->vel.y = e->vel.y*-0.3f;
	}
	if((col&0x00F) && (e->vel.y < 0.f)){
		e->flags &= ~ENTITY_FALLING;
		if(e->vel.y < -0.15f){
			e->yoff = -0.8f;
			ret += (int)(fabsf(e->vel.y)*128.f);
		}else if(e->vel.y < -0.07f){
			e->yoff += -0.4f;
		}else if(e->vel.y < -0.04f){
			e->yoff += -0.2f;
		}
		e->vel = vecMul(e->vel,vecNew(0.93f,0,0.93f));
		e->vel.y = 0.001f;
	}

	entityUpdateCurChungus(e);
	return ret;
}

float entityDistance(const entity *e, const character *c){
	return vecMag(vecSub(e->pos,c->pos));
}

uint lineOfSightBlockCount(const vec a, const vec b, uint maxB){
	float lastDist = 999999.f;
	uint ret = 0;
	vec p = a;
	ivec lastB = ivecNOne();
	vec dist = vecSub(b,p);
	if(vecMag(dist) > 128.f){return maxB+1;}

	for(uint max=1<<10;max;--max){
		dist = vecSub(b,p);
		const float curDist = vecMag(dist);
		if(curDist > lastDist){break;}
		lastDist = curDist;
		const vec v = vecMulS(vecNorm(dist),0.5f);
		p = vecAdd(p,v);
		const ivec newB = ivecNewV(p);
		if(!ivecEq(lastB,newB) && worldGetB(newB.x,newB.y,newB.z)){
			lastB = newB;
			if(++ret > maxB){return ret;}
		}

	}
	return ret;
}
