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
#include "rope.h"
#include "character/character.h"
#include "entity.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../../../common/src/game/being.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <string.h>

mesh *ropeMesh = NULL;

int ropeNewID(){
	if(playerID < 0){return -1;}
	const uint start = playerID << 2;
	for(uint i=start;i<start+4;i++){
		if(ropeList[i].a == 0){return i;}
		if(ropeList[i].b == 0){return i;}
	}
	return -1;
}

static void ropeDrawSegment(const rope *r, const vec h, const vec p){
	const float rlen = vecMag(vecSub(h,p))*8;
	const float xs = 1.f/2.f;
	float xo = 0.f;
	switch(r->flags & ROPE_TEX){
	default:
	case ROPE_TEX_ROPE:
		xo = 0.f;
		break;
	case ROPE_TEX_CHAIN:
		xo = 1.f * xs;
		break;
	}

	meshAddVert(ropeMesh,p.x-.05f,p.y,p.z,xo   ,     .5f);
	meshAddVert(ropeMesh,p.x+.05f,p.y,p.z,xo+xs,     .5f);
	meshAddVert(ropeMesh,h.x+.05f,h.y,h.z,xo+xs,rlen+.5f);

	meshAddVert(ropeMesh,h.x+.05f,h.y,h.z,xo+xs,rlen+.5f);
	meshAddVert(ropeMesh,h.x-.05f,h.y,h.z,xo   ,rlen+.5f);
	meshAddVert(ropeMesh,p.x-.05f,p.y,p.z,xo   ,     .5f);

	meshAddVert(ropeMesh,p.x-.05f,p.y,p.z,xo   ,     .5f);
	meshAddVert(ropeMesh,h.x+.05f,h.y,h.z,xo+xs,rlen+.5f);
	meshAddVert(ropeMesh,p.x+.05f,p.y,p.z,xo+xs,     .5f);

	meshAddVert(ropeMesh,h.x+.05f,h.y,h.z,xo+xs,rlen+.5f);
	meshAddVert(ropeMesh,p.x-.05f,p.y,p.z,xo   ,     .5f);
	meshAddVert(ropeMesh,h.x-.05f,h.y,h.z,xo   ,rlen+.5f);


	meshAddVert(ropeMesh,p.x,p.y-.05f,p.z,xo   , 0.f);
	meshAddVert(ropeMesh,p.x,p.y+.05f,p.z,xo+xs, 0.f);
	meshAddVert(ropeMesh,h.x,h.y+.05f,h.z,xo+xs,rlen);

	meshAddVert(ropeMesh,h.x,h.y+.05f,h.z,xo+xs,rlen);
	meshAddVert(ropeMesh,h.x,h.y-.05f,h.z,xo   ,rlen);
	meshAddVert(ropeMesh,p.x,p.y-.05f,p.z,xo   , 0.f);

	meshAddVert(ropeMesh,p.x,p.y-.05f,p.z,xo   , 0.f);
	meshAddVert(ropeMesh,h.x,h.y+.05f,h.z,xo+xs,rlen);
	meshAddVert(ropeMesh,p.x,p.y+.05f,p.z,xo+xs, 0.f);

	meshAddVert(ropeMesh,h.x,h.y+.05f,h.z,xo+xs,rlen);
	meshAddVert(ropeMesh,p.x,p.y-.05f,p.z,xo  , 0.f);
	meshAddVert(ropeMesh,h.x,h.y-.05f,h.z,xo  ,rlen);


	meshAddVert(ropeMesh,p.x,p.y,p.z-.05f,xo   , 0.f);
	meshAddVert(ropeMesh,p.x,p.y,p.z+.05f,xo+xs, 0.f);
	meshAddVert(ropeMesh,h.x,h.y,h.z+.05f,xo+xs,rlen);

	meshAddVert(ropeMesh,h.x,h.y,h.z+.05f,xo+xs,rlen);
	meshAddVert(ropeMesh,h.x,h.y,h.z-.05f,xo   ,rlen);
	meshAddVert(ropeMesh,p.x,p.y,p.z-.05f,xo   , 0.f);

	meshAddVert(ropeMesh,p.x,p.y,p.z-.05f,xo   , 0.f);
	meshAddVert(ropeMesh,h.x,h.y,h.z+.05f,xo+xs,rlen);
	meshAddVert(ropeMesh,p.x,p.y,p.z+.05f,xo+xs, 0.f);

	meshAddVert(ropeMesh,h.x,h.y,h.z+.05f,xo+xs,rlen);
	meshAddVert(ropeMesh,p.x,p.y,p.z-.05f,xo   , 0.f);
	meshAddVert(ropeMesh,h.x,h.y,h.z-.05f,xo   ,rlen);
}

static vec beingRopeOffset(being b){
	switch(beingType(b)){
	case BEING_CHARACTER: {
		const character *c = characterGetByBeing(b);
		return vecNew(0.3 * cosf((c->rot.yaw + 180) * PI180), -0.2, 0.3 * sinf((c->rot.yaw + 180) * PI180)); }
	default:
		return vecZero();
	}
}

static void ropeDraw(rope *r){
	if(r == NULL){return;}
	const vec h = beingGetPos(r->a);
	const vec p = beingGetPos(r->b);
	if((h.y < 0.f) || (p.y < 0.f)){return;}

	ropeDrawSegment(r,vecSub(vecAdd(beingRopeOffset(r->a),h),subBlockViewOffset),vecSub(vecAdd(beingRopeOffset(r->b),p),subBlockViewOffset));
}

void ropeInit(){
	ropeMesh = meshNew(NULL);
	ropeMesh->tex = tRope;
	memset(ropeList,0,sizeof(ropeList));
}

void ropeDrawAll(){
	meshEmpty(ropeMesh);
	for(uint i=0;i<512;i++){
		rope *r = &ropeList[i];
		if((r->a == 0) || (r->b == 0)){continue;}
		ropeDraw(r);
	}
	if(ropeMesh->dataCount == 0){return;}
	shaderBind(sMesh);
	matMul(matMVP,matSubBlockView,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshFinishDynamic(ropeMesh);
	meshDraw(ropeMesh);
}

void ropeUpdateP(const packet *p){
	const uint i = p->v.u32[0];
	int rclient = ropeGetClient(i);
	if(rclient < 0)         {return;}
	if(rclient == playerID) {return;}
	rope *r = &ropeList[i];
	r->a      = p->v.u32[1];
	r->b      = p->v.u32[2];
	r->flags  = p->v.u32[3];
	r->length = p->v.f  [4];

	r->flags |= ROPE_DIRTY;
}

void ropeSyncAll(){
	const uint start = playerID << 2;
	for(uint i=start;i<start+4;i++){
		if(!(ropeList[i].flags & ROPE_DIRTY)){continue;}
		msgRopeUpdate(-1, i, &ropeList[i]);
		ropeList[i].flags &= ~ROPE_DIRTY;
	}
}

void ropeDelBeing(const being t){
	if(playerID < 0){return;}
	const uint start = playerID << 2;
	for(uint i = start;i<start+4;i++){
		if((ropeList[i].a == t) || (ropeList[i].b == t)){
			ropeList[i].flags = 0;
			ropeList[i].a     = 0;
			ropeList[i].b     = 0;
		}
	}
}
