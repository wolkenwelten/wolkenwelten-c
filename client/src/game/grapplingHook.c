#include "../game/grapplingHook.h"
#include "../main.h"
#include "../game/blockType.h"
#include "../game/entity.h"
#include "../gfx/mat.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/mesh.h"
#include "../gfx/effects.h"
#include "../gfx/texture.h"
#include "../sdl/sfx.h"
#include "../tmp/assets.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../gfx/glew.h"

grapplingHook grapplingHookList[128];
int grapplingHookCount = 0;

grapplingHook *grapplingHookNew(character *p){
	grapplingHook *ghk = NULL;;
	for(int i=0;i<grapplingHookCount;i++){
		if(grapplingHookList[i].parent == NULL){
			ghk = &grapplingHookList[i];
			break;
		}
	}
	if(ghk == NULL){
		ghk = &grapplingHookList[grapplingHookCount++];
	}

	ghk->parent = p;
	ghk->ent = entityNew(p->x,p->y+1.f,p->z,-p->yaw,-p->pitch-90.f,p->roll);
	ghk->ent->vx = p->vx + 1.2f * (cos((p->yaw-90.f)*PI/180) * cos(-p->pitch*PI/180));
	ghk->ent->vy = p->vy + 1.2f *  sin(-p->pitch*PI/180);
	ghk->ent->vz = p->vz + 1.2f * (sin((p->yaw-90.f)*PI/180) * cos(-p->pitch*PI/180));
	ghk->ent->eMesh = meshHook;
	ghk->ent->noRepulsion = 1;
	ghk->rope = meshNew();
	ghk->rope->tex = tRope;

	ghk->hooked     = false;
	ghk->returning  = false;
	ghk->goalLength = 0;

	return ghk;
}

void grapplingHookFree(grapplingHook *ghk){
	entityFree(ghk->ent);
	ghk->ent = NULL;
	meshFree(ghk->rope);
	ghk->parent = NULL;
	ghk->rope = NULL;
}

void grapplingHookUpdateRope(grapplingHook *ghk){
	float hx,hy,hz,px,py,pz;
	mesh *m = ghk->rope;
	hx = ghk->ent->x;
	hy = ghk->ent->y;
	hz = ghk->ent->z;
	px = ghk->parent->x;
	py = ghk->parent->y+ghk->ent->yoff+.1f;
	pz = ghk->parent->z;
	const float rlen = grapplingHookGetLength(ghk)*8;
	m->dataCount = 0;

	meshAddVert(m,px-.05f,py,pz,0.f, 0.f);
	meshAddVert(m,px+.05f,py,pz,1.f, 0.f);
	meshAddVert(m,hx+.05f,hy,hz,1.f,rlen);

	meshAddVert(m,hx+.05f,hy,hz,1.f,rlen);
	meshAddVert(m,hx-.05f,hy,hz,0.f,rlen);
	meshAddVert(m,px-.05f,py,pz,0.f, 0.f);

	meshAddVert(m,px-.05f,py,pz,0.f, 0.f);
	meshAddVert(m,hx+.05f,hy,hz,1.f,rlen);
	meshAddVert(m,px+.05f,py,pz,1.f, 0.f);

	meshAddVert(m,hx+.05f,hy,hz,1.f,rlen);
	meshAddVert(m,px-.05f,py,pz,0.f, 0.f);
	meshAddVert(m,hx-.05f,hy,hz,0.f,rlen);


	meshAddVert(m,px,py-.05f,pz,0.f, 0.f);
	meshAddVert(m,px,py+.05f,pz,1.f, 0.f);
	meshAddVert(m,hx,hy+.05f,hz,1.f,rlen);

	meshAddVert(m,hx,hy+.05f,hz,1.f,rlen);
	meshAddVert(m,hx,hy-.05f,hz,0.f,rlen);
	meshAddVert(m,px,py-.05f,pz,0.f, 0.f);

	meshAddVert(m,px,py-.05f,pz,0.f, 0.f);
	meshAddVert(m,hx,hy+.05f,hz,1.f,rlen);
	meshAddVert(m,px,py+.05f,pz,1.f, 0.f);

	meshAddVert(m,hx,hy+.05f,hz,1.f,rlen);
	meshAddVert(m,px,py-.05f,pz,0.f, 0.f);
	meshAddVert(m,hx,hy-.05f,hz,0.f,rlen);


	meshAddVert(m,px,py,pz-.05f,0.f, 0.f);
	meshAddVert(m,px,py,pz+.05f,1.f, 0.f);
	meshAddVert(m,hx,hy,hz+.05f,1.f,rlen);

	meshAddVert(m,hx,hy,hz+.05f,1.f,rlen);
	meshAddVert(m,hx,hy,hz-.05f,0.f,rlen);
	meshAddVert(m,px,py,pz-.05f,0.f, 0.f);

	meshAddVert(m,px,py,pz-.05f,0.f, 0.f);
	meshAddVert(m,hx,hy,hz+.05f,1.f,rlen);
	meshAddVert(m,px,py,pz+.05f,1.f, 0.f);

	meshAddVert(m,hx,hy,hz+.05f,1.f,rlen);
	meshAddVert(m,px,py,pz-.05f,0.f, 0.f);
	meshAddVert(m,hx,hy,hz-.05f,0.f,rlen);

	meshFinish(ghk->rope, GL_STREAM_DRAW);
}

float grapplingHookGetLength(grapplingHook *ghk){
	float dx = ghk->ent->x - ghk->parent->x;
	float dy = ghk->ent->y - ghk->parent->y;
	float dz = ghk->ent->z - ghk->parent->z;
	return sqrtf((dx*dx)+(dy*dy)+(dz*dz));
}

bool grapplingHookReturnToParent(grapplingHook *ghk,float speed){
	float dx = ghk->ent->x - ghk->parent->x;
	float dy = ghk->ent->y - ghk->parent->y;
	float dz = ghk->ent->z - ghk->parent->z;
	float d = sqrtf((dx*dx)+(dy*dy)+(dz*dz));
	speed = d / 8.f;
	if(speed < 1.f){speed = 1.f;}
	float dm = fabsf(dx);
	if(fabsf(dy) > dm){dm = fabsf(dy);}
	if(fabsf(dz) > dm){dm = fabsf(dz);}
	if(dm <= 3.1f){
		return true;
	}
	dx /= dm;
	dy /= dm;
	dz /= dm;
	ghk->ent->vx = (dx * -speed);
	ghk->ent->vy = (dy * -speed);
	ghk->ent->vz = (dz * -speed);

	if(ghk->ent->collide){
		ghk->ent->x += dx;
		ghk->ent->y += dy;
		ghk->ent->z += dz;
	}
	return false;
}

void grapplingHookReturnHook(grapplingHook *ghk){
	ghk->returning = true;
	ghk->hooked = false;
	ghk->ent->vx = 0.f;
	ghk->ent->vy = 0.f;
	ghk->ent->vz = 0.f;
	grapplingHookReturnToParent(ghk,0.1f);
}

void grapplingHookPullTowards(grapplingHook *ghk,character *pull){
	float dx = ghk->ent->x - ghk->parent->x;
	float dy = ghk->ent->y - ghk->parent->y;
	float dz = ghk->ent->z - ghk->parent->z;
	float dist = ghk->goalLength - sqrtf((dx*dx)+(dy*dy)+(dz*dz));
	float vx = pull->vx;
	float vy = pull->vy;
	float vz = pull->vz;

	float dm = fabsf(dx);
	if(fabsf(dy) > dm){dm = fabsf(dy);}
	if(fabsf(dz) > dm){dm = fabsf(dz);}
	dx = (dx / dm) * dist;
	dy = (dy / dm) * dist;
	dz = (dz / dm) * dist;

	uint32_t col = entityCollision(pull->x,pull->y,pull->z);
	if(((dx > 0) && !((col & 0x110))) || ((dx < 0) && !((col & 0x220)))){
		pull->x -= dx;
		vx -= dx*0.2f;
	}
	if(((dz > 0) && !((col & 0x440))) || ((dz < 0) && !((col & 0x880)))){
		pull->z -= dz;
		vz -= dz*0.2f;
	}
	if(((dy > 0) && !((col & 0x00F))) || ((dy < 0) && !((col & 0x0F0)))){
		pull->y -= dy;
		vy -= dy*0.2f;
	}
	pull->vx = vx;
	pull->vy = vy;
	pull->vz = vz;
}

bool grapplingHookUpdate(grapplingHook *ghk){
	entityUpdate(ghk->ent);

	if(ghk->returning && grapplingHookReturnToParent(ghk,0.01f)){
		return true;
	}else if(ghk->hooked && !ghk->returning){
		ghk->ent->vx = 0.f;
		ghk->ent->vy = 0.f;
		ghk->ent->vz = 0.f;
		if(!ghk->ent->collide){
			grapplingHookReturnHook(ghk);
		}
	}else{
		if(!ghk->hooked && !ghk->returning && ghk->ent->collide){
			ghk->ent->vx = 0.f;
			ghk->ent->vy = 0.f;
			ghk->ent->vz = 0.f;
			ghk->ent->noClip = 1;
			ghk->hooked = true;
			ghk->goalLength = grapplingHookGetLength(ghk);
			sfxPlay(sfxHookHit,1.f);
			sfxLoop(sfxHookRope,0.f);
			unsigned char b = worldGetB(ghk->ent->x,ghk->ent->y,ghk->ent->z);
			if(b){
				fxBlockBreak(ghk->ent->x,ghk->ent->y,ghk->ent->z,b);
			}
		}else if(!ghk->hooked && !ghk->returning){
			sfxLoop(sfxHookRope,1.f);
			if(grapplingHookGetLength(ghk) > 256.f){
				grapplingHookReturnHook(ghk);
			}
		}
	}
	if(ghk->ent->y < -256.f){
		return true;
	}
	return false;
}

void grapplingHookDrawRopes(){
	float matMVP[16];
	if(grapplingHookCount == 0){return;}
	shaderBind(sMesh);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sMesh,matMVP);
	for(int i=0;i<grapplingHookCount;i++){
		if(grapplingHookList[i].parent == NULL){continue;}
		grapplingHookUpdateRope(&grapplingHookList[i]);
		meshDraw(grapplingHookList[i].rope);
	}
}

bool grapplingHookGetHooked(grapplingHook *ghk){
	return ghk->hooked;
}
float grapplingHookGetGoalLength(grapplingHook *ghk){
	return ghk->goalLength;
}
void grapplingHookSetGoalLength(grapplingHook *ghk, float len){
	ghk->goalLength = len;
}
