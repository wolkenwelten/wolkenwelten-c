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
#include "../gfx/gl.h"

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

	ghk->parent           = p;
	ghk->ent              = entityNew(vecAdd(vecNew(0,1,0),p->pos),vecNew(-p->rot.yaw,-p->rot.pitch-90.f,p->rot.roll));
	ghk->ent->vel.x       = p->vel.x + 1.2f * (cos((p->rot.yaw-90.f)*PI/180) * cos(-p->rot.pitch*PI/180));
	ghk->ent->vel.y       = p->vel.y + 1.2f *  sin(-p->rot.pitch*PI/180);
	ghk->ent->vel.z       = p->vel.z + 1.2f * (sin((p->rot.yaw-90.f)*PI/180) * cos(-p->rot.pitch*PI/180));
	ghk->ent->eMesh       = meshHook;
	ghk->ent->flags       = ENTITY_NOREPULSION;
	ghk->rope             = meshNew();
	ghk->rope->tex        = tRope;

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
	hx = ghk->ent->pos.x;
	hy = ghk->ent->pos.y;
	hz = ghk->ent->pos.z;
	px = ghk->parent->pos.x;
	py = ghk->parent->pos.y+ghk->ent->yoff+.1f;
	pz = ghk->parent->pos.z;
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

float grapplingHookGetLength(const grapplingHook *ghk){
	return vecMag(vecSub(ghk->ent->pos,ghk->parent->pos));
}

bool grapplingHookReturnToParent(grapplingHook *ghk,float speed){
	vec dist = vecSub(ghk->ent->pos,ghk->parent->pos);
	float d  = vecMag(dist);
	speed = d / 8.f;
	if(speed < 1.f){speed = 1.f;}
	if(d <= 2){return true;}
	ghk->ent->vel = vecMulS(vecNorm(dist),-speed);

	if(ghk->ent->flags & ENTITY_COLLIDE){
		ghk->ent->pos = vecAdd(ghk->ent->pos,ghk->ent->vel);
	}
	return false;
}

void grapplingHookReturnHook(grapplingHook *ghk){
	ghk->returning = true;
	ghk->hooked    = false;
	ghk->ent->vel  = vecZero();
	grapplingHookReturnToParent(ghk,0.1f);
}

void grapplingHookPullTowards(grapplingHook *ghk,character *pull){
	float  len  = grapplingHookGetLength(ghk);
	const vec d = vecMulS(vecNorm(vecSub(ghk->ent->pos,ghk->parent->pos)),ghk->goalLength - len);

	u32 col = entityCollision(pull->pos);
	if(((d.x > 0) && !((col & 0x110))) || ((d.x < 0) && !((col & 0x220)))){
		pull->pos.x -= d.x;
		pull->vel.x -= d.x*0.2f;
	}
	if(((d.z > 0) && !((col & 0x440))) || ((d.z < 0) && !((col & 0x880)))){
		pull->pos.z -= d.z;
		pull->vel.z -= d.z*0.2f;
	}
	if(((d.y > 0) && !((col & 0x00F))) || ((d.y < 0) && !((col & 0x0F0)))){
		pull->pos.y -= d.y;
		pull->vel.y -= d.y*0.2f;
	}
	pull->shake = vecMag(pull->vel);
}

bool grapplingHookUpdate(grapplingHook *ghk){
	entityUpdate(ghk->ent);

	if(ghk->returning && grapplingHookReturnToParent(ghk,0.01f)){
		return true;
	}else if(ghk->hooked && !ghk->returning){
		ghk->ent->vel = vecZero();
		if(!(ghk->ent->flags & ENTITY_COLLIDE)){
			grapplingHookReturnHook(ghk);
		}
	}else{
		if(!ghk->hooked && !ghk->returning && (ghk->ent->flags & ENTITY_COLLIDE)){
			ghk->ent->vel = vecZero();
			ghk->ent->flags |= ENTITY_NOCLIP;
			ghk->hooked = true;
			ghk->goalLength = grapplingHookGetLength(ghk);
			sfxPlay(sfxHookHit,1.f);
			sfxLoop(sfxHookRope,0.f);
			unsigned char b = worldGetB(ghk->ent->pos.x,ghk->ent->pos.y,ghk->ent->pos.z);
			if(b){
				fxBlockBreak(ghk->ent->pos,b);
			}
		}else if(!ghk->hooked && !ghk->returning){
			sfxLoop(sfxHookRope,1.f);
			if(grapplingHookGetLength(ghk) > 256.f){
				grapplingHookReturnHook(ghk);
			}
		}
	}
	if(ghk->ent->pos.y < -256.f){
		return true;
	}
	return false;
}

void grapplingHookDrawRopes(){
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

bool grapplingHookGetHooked(const grapplingHook *ghk){
	return ghk->hooked;
}
float grapplingHookGetGoalLength(const grapplingHook *ghk){
	return ghk->goalLength;
}
void grapplingHookSetGoalLength(grapplingHook *ghk, float len){
	ghk->goalLength = len;
}
