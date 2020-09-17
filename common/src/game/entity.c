#include "entity.h"

#include "../mods/api_v1.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

void entityReset(entity *e){
	memset(e,0,sizeof(entity));
}

uint32_t entityCollision(const vec c){
	uint32_t col = 0;

	if(checkCollision(c.x-0.3f,c.y     ,c.z     )){col |= 0x100;}
	if(checkCollision(c.x+0.3f,c.y     ,c.z     )){col |= 0x200;}
	if(checkCollision(c.x     ,c.y     ,c.z-0.3f)){col |= 0x400;}
	if(checkCollision(c.x     ,c.y     ,c.z+0.3f)){col |= 0x800;}
	if(checkCollision(c.x     ,c.y+0.5f,c.z     )){col |=  0xF0;}
	if(checkCollision(c.x     ,c.y-0.5f,c.z     )){col |=   0xF;}

	return col;
}

void entityUpdateCurChungus(entity *e){
	const int cx = (int)e->x >> 8;
	const int cy = (int)e->y >> 8;
	const int cz = (int)e->z >> 8;
	e->curChungus = worldTryChungus(cx,cy,cz);
}

int entityUpdate(entity *e){
	int ret=0;
	uint32_t col;
	e->pos = vecAdd(e->pos,e->vel);
	if(e->flags & ENTITY_NOCLIP){
		e->flags &= ~ENTITY_COLLIDE;
		if(entityCollision(e->x,e->y,e->z)){ e->flags |= ENTITY_COLLIDE; }
		entityUpdateCurChungus(e);
		return 0;
	}

	e->vel.y -= 0.0005f;
	// ToDo: implement terminal veolocity in a better way
	if(e->vel.y < -1.0f){e->vel.y+=0.005f;}
	if(e->vel.y >  1.0f){e->vel.y-=0.005f;}

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
		if(e->vx >  0.1f){ ret += (int)(fabsf(e->vx)*128.f); }
		e->pos.x = MIN(e->pos.x,floorf(e->x)+0.7f);
		e->vel.x = e->vel.x*-0.3f;
	}
	if((col&0x880) && (e->vel.z > 0.f)){
		if(e->vel.z >  0.1f){ ret += (int)(fabsf(e->vel.z)*128.f); }
		e->pos.z = MIN(e->pos.z,floorf(e->z)+0.7f);
		e->vel.z = e->vel.z*-0.3f;
	}
	if((col&0x440) && (e->vel.z < 0.f)){
		if(e->vel.z < -0.1f){ ret += (int)(fabsf(e->vel.z)*128.f); }
		e->pos.z = MAX(e->pos.z,floorf(e->z)+0.3f);
		e->vel.z = e->vel.z*-0.3f;
	}
	if((col&0x0F0) && (e->vel.y > 0.f)){
		if(e->vel.y >  0.1f){ ret += (int)(fabsf(e->vel.y)*128.f); }
		e->pos.y = MIN(e->pos.y,floorf(e->y)+0.5f);
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
		e->vel = vecMul(e->vel,vecNew(0.97f,0,0.97f);
	}

	entityUpdateCurChungus(e);
	return ret;
}

float entityDistance(entity *e, character *c){
	return vecMag(vecSub(e->pos,c->pos));
}
