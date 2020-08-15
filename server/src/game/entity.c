#include "entity.h"

#include "../main.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>

entity entityList[1<<14];
int entityCount = 0;
entity *entityFirstFree = NULL;


void entityReset(entity *e){
	e->x   = e->y     = e->z    = 0.f;
	e->vx  = e->vy    = e->vz   = 0.f;
	e->yaw = e->pitch = e->roll = 0.f;
	e->yoff                     = 0.f;
	e->falling                  = false;
	e->noClip                   = false;
	e->updated                  = false;
	e->collide                  = false;
	e->noRepulsion              = false;
	e->nextFree                 = NULL;
}


entity *entityNew(float x, float y, float z , float yaw, float pitch, float roll){
	entity *e = NULL;
	if(entityFirstFree == NULL){
		e = &entityList[entityCount++];
	}else{
		e = entityFirstFree;
		entityFirstFree = (entity *)e->nextFree;
	}

	entityReset(e);
	e->nextFree = NULL;
	e->x = x;
	e->y = y;
	e->z = z;
	e->yaw   = yaw;
	e->pitch = pitch;
	e->roll  = roll;
	return e;
}

void entityFree(entity *e){
	if(e == NULL){return;}
	e->nextFree = entityFirstFree;
	entityFirstFree = e;
}

uint32_t entityCollision(float cx, float cy, float cz){
	uint32_t col = 0;

	if(checkCollision(cx   ,cy+0.5f,cz   )){col |= 0xFF0;}
	if(checkCollision(cx   ,cy-0.5f,cz   )){col |= 0xF0F;}

	return col;
}

int entityUpdate(entity *e){
	int ret=0;
	uint32_t col;
	e->x += e->vx;
	e->y += e->vy;
	e->z += e->vz;
	if(e->noClip){
		e->collide = entityCollision(e->x,e->y,e->z);
		return 0;
	}

	e->vy -= 0.0005f;
	if(e->vy < -1.0f){e->vy+=0.005f;}
	if(e->vy >  1.0f){e->vy-=0.005f;}

	e->falling = true;
	e->collide = false;
	col = entityCollision(e->x,e->y,e->z);
	if(col){ e->collide = true; }
	e->updated = true;
	if(e->noRepulsion){ return 0; }
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
		e->x = floorf(e->x)+0.7f;
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
		e->falling=false;
		if(e->vy < -0.15f){
			e->yoff = -0.8f;
			ret += (int)(fabsf(e->vy)*128.f);
		}else if(e->vy < -0.07f){
			e->yoff += -0.4f;
		}else if(e->vy < -0.04f){
			e->yoff += -0.2f;
		}
		e->vx *= 0.97f;
		e->vy = 0.f;
		e->vz *= 0.97f;
	}

	return ret;
}

void entityUpdateAll(){
	for(int i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(entityList[i].updated){
			entityList[i].updated = false;
			continue;
		}
		entityUpdate(&entityList[i]);
		entityList[i].updated = false;
	}
}
