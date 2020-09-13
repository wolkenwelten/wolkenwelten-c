#include "animal.h"

#include "../mods/api_v1.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void animalReset(animal *e){
	memset(e,0,sizeof(animal));
}

uint32_t animalCollision(float cx, float cy, float cz){
	uint32_t col = 0;

	if(checkCollision(cx-0.3f,cy     ,cz     )){col |= 0x100;}
	if(checkCollision(cx+0.3f,cy     ,cz     )){col |= 0x200;}
	if(checkCollision(cx     ,cy     ,cz-0.3f)){col |= 0x400;}
	if(checkCollision(cx     ,cy     ,cz+0.3f)){col |= 0x800;}
	if(checkCollision(cx     ,cy+0.5f,cz     )){col |= 0x0F0;}
	if(checkCollision(cx     ,cy-1.0f,cz     )){col |= 0x00F;}

	return col;
}

void animalUpdateCurChungus(animal *e){
	const int cx = (int)e->x >> 8;
	const int cy = (int)e->y >> 8;
	const int cz = (int)e->z >> 8;
	e->curChungus = worldTryChungus(cx,cy,cz);
}

int animalUpdate(animal *e){
	int ret=0;
	uint32_t col;
	e->x += e->vx;
	e->y += e->vy;
	e->z += e->vz;
	e->breathing += 5;
	
	if(fabsf(e->yaw - e->gyaw) > 0.3f){
		if(e->yaw > e->gyaw){
			e->yaw -= 0.1f;
		}else{
			e->yaw += 0.1f;
		}
	}
	if(fabsf(e->pitch - e->gpitch) > 0.3f){
		if(e->pitch > e->gpitch){
			e->pitch -= 0.1f;
		}else{
			e->pitch += 0.1f;
		}
	}
	
	if(fabsf(e->vx - e->gvx) > 0.001f){
		if(e->vx > e->gvx){
			e->vx -= 0.005f;
		}else{
			e->vx += 0.005f;
		}
	}
	if(fabsf(e->vz - e->gvz) > 0.001f){
		if(e->vz > e->gvz){
			e->vz -= 0.005f;
		}else{
			e->vz += 0.005f;
		}
	}
	
	if(e->yaw > 360.f){e->yaw -= 360.f;}
	if(e->pitch > 180.f){e->pitch -= 360.f;}
	
	if(e->yoff < 0.f){
		e->yoff += 0.003f;
	}else{
		e->yoff -= 0.003f;
	}

	e->vy -= 0.0005f;
	// ToDo: implement terminal veolocity in a better way
	if(e->vy < -1.0f){e->vy+=0.005f;}
	if(e->vy >  1.0f){e->vy-=0.005f;}

	e->flags |=  ANIMAL_FALLING;
	e->flags &= ~ANIMAL_COLLIDE;
	col = animalCollision(e->x,e->y,e->z);
	if(col){ e->flags |= ANIMAL_COLLIDE; }

	if((col&0x110) && (e->vx < 0.f)){
		if(e->vx < -0.05f){ ret += (int)(fabsf(e->vx)*24.f); }
		const float nx = floor(e->x)+0.3f;
		if(nx > e->x){e->x = nx;}
		e->vx = e->vx*-0.3f;
	}
	if((col&0x220) && (e->vx > 0.f)){
		if(e->vx >  0.05f){ ret += (int)(fabsf(e->vx)*24.f); }
		const float nx = floorf(e->x)+0.7f;
		if(nx < e->x){e->x = nx;}
		e->vx = e->vx*-0.3f;
	}
	if((col&0x880) && (e->vz > 0.f)){
		if(e->vz >  0.05f){ ret += (int)(fabsf(e->vz)*24.f); }
		const float nz = floorf(e->z)+0.7f;
		if(nz < e->z){e->z = nz;}
		e->vz = e->vz*-0.3f;
	}
	if((col&0x440) && (e->vz < 0.f)){
		if(e->vz < -0.05f){ ret += (int)(fabsf(e->vz)*24.f); }
		const float nz = floorf(e->z)+0.3f;
		if(nz > e->z){e->z = nz;}
		e->vz = e->vz*-0.3f;
	}
	if((col&0x0F0) && (e->vy > 0.f)){
		if(e->vy >  0.05f){ ret += (int)(fabsf(e->vy)*24.f); }
		const float ny = floorf(e->y)+0.5f;
		if(ny < e->y){e->y = ny;}
		e->vy = e->vy*-0.3f;
	}
	if((col&0x00F) && (e->vy < 0.f)){
		e->flags &= ~ANIMAL_FALLING;
		if(e->vy < -0.05f){
			ret += (int)(fabsf(e->vy)*24.f);
		}
		e->vx *= 0.97f;
		e->vy  = 0.00f;
		e->vz *= 0.97f;
	}

	animalUpdateCurChungus(e);
	return ret;
}

float animalDistance(animal *e, character *c){
	const float dx = e->x - c->x;
	const float dy = e->y - c->y;
	const float dz = e->z - c->z;
	return (dx*dx)+(dy*dy)+(dz*dz);
}
