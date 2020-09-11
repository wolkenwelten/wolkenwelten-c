#include "entity.h"

#include "../mods/api_v1.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

void entityReset(entity *e){
	memset(e,0,sizeof(entity));
}

uint32_t entityCollision(float cx, float cy, float cz){
	uint32_t col = 0;

	if(checkCollision(cx-0.3f,cy     ,cz     )){col |= 0x100;}
	if(checkCollision(cx+0.3f,cy     ,cz     )){col |= 0x200;}
	if(checkCollision(cx     ,cy     ,cz-0.3f)){col |= 0x400;}
	if(checkCollision(cx     ,cy     ,cz+0.3f)){col |= 0x800;}
	if(checkCollision(cx     ,cy+0.5f,cz     )){col |=  0xF0;}
	if(checkCollision(cx     ,cy-0.5f,cz     )){col |=   0xF;}

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
	e->x += e->vx;
	e->y += e->vy;
	e->z += e->vz;
	if(e->flags & ENTITY_NOCLIP){
		e->flags &= ~ENTITY_COLLIDE;
		if(entityCollision(e->x,e->y,e->z)){ e->flags |= ENTITY_COLLIDE; }
		entityUpdateCurChungus(e);
		return 0;
	}

	e->vy -= 0.0005f;
	// ToDo: implement terminal veolocity in a better way
	if(e->vy < -1.0f){e->vy+=0.005f;}
	if(e->vy >  1.0f){e->vy-=0.005f;}

	e->flags |=  ENTITY_FALLING;
	e->flags &= ~ENTITY_COLLIDE;
	col = entityCollision(e->x,e->y,e->z);
	if(col){ e->flags |= ENTITY_COLLIDE; }
	e->flags |= ENTITY_UPDATED;
	if(e->flags & ENTITY_NOREPULSION){
		entityUpdateCurChungus(e);
		return 0;
	}
	if((col&0x110) && (e->vx < 0.f)){
		if(e->vx < -0.1f){ ret += (int)(fabsf(e->vx)*128.f); }
		const float nx = floor(e->x)+0.3f;
		if(nx > e->x){e->x = nx;}
		e->vx = e->vx*-0.3f;
	}
	if((col&0x220) && (e->vx > 0.f)){
		if(e->vx >  0.1f){ ret += (int)(fabsf(e->vx)*128.f); }
		const float nx = floorf(e->x)+0.7f;
		if(nx < e->x){e->x = nx;}
		e->vx = e->vx*-0.3f;
	}
	if((col&0x880) && (e->vz > 0.f)){
		if(e->vz >  0.1f){ ret += (int)(fabsf(e->vz)*128.f); }
		const float nz = floorf(e->z)+0.7f;
		if(nz < e->z){e->z = nz;}
		e->vz = e->vz*-0.3f;
	}
	if((col&0x440) && (e->vz < 0.f)){
		if(e->vz < -0.1f){ ret += (int)(fabsf(e->vz)*128.f); }
		const float nz = floorf(e->z)+0.3f;
		if(nz > e->z){e->z = nz;}
		e->vz = e->vz*-0.3f;
	}
	if((col&0x0F0) && (e->vy > 0.f)){
		if(e->vy >  0.1f){ ret += (int)(fabsf(e->vy)*128.f); }
		const float ny = floorf(e->y)+0.5f;
		if(ny < e->y){e->y = ny;}
		e->vy = e->vy*-0.3f;
	}
	if((col&0x00F) && (e->vy < 0.f)){
		e->flags &= ~ENTITY_FALLING;
		if(e->vy < -0.15f){
			e->yoff = -0.8f;
			ret += (int)(fabsf(e->vy)*128.f);
		}else if(e->vy < -0.07f){
			e->yoff += -0.4f;
		}else if(e->vy < -0.04f){
			e->yoff += -0.2f;
		}
		e->vx *= 0.97f;
		e->vy  = 0.f;
		e->vz *= 0.97f;
	}

	entityUpdateCurChungus(e);
	return ret;
}

float entityDistance(entity *e, character *c){
	const float dx = e->x - c->x;
	const float dy = e->y - c->y;
	const float dz = e->z - c->z;
	return (dx*dx)+(dy*dy)+(dz*dz);
}
