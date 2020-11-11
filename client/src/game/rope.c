#include "rope.h"

#include "../game/entity.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../../../common/src/game/being.h"
#include "../../../common/src/network/messages.h"

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
	if(h.y < 0.f){return;}
	if(p.y < 0.f){return;}
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

static void ropeDraw(rope *r){
	if(r == NULL){return;}
	const vec h = beingGetPos(r->a);
	const vec p = beingGetPos(r->b);
	ropeDrawSegment(r,h,p);
}

void ropeInit(){
	ropeMesh = meshNew();
	ropeMesh->tex = tRope;
	memset(ropeList,0,sizeof(ropeList));
}

void ropeDrawAll(){
	shaderBind(sMesh);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshEmpty(ropeMesh);
	for(uint i=0;i<512;i++){
		rope *r = &ropeList[i];
		if(r->a == 0){continue;}
		if(r->b == 0){continue;}
		ropeDraw(r);
	}
	meshFinishStream(ropeMesh);
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

	r->flags |= ROPE_UPDATED;
}

void ropeSyncAll(){
	const uint start = playerID << 2;
	for(uint i=start;i<start+4;i++){
		if(ropeList[i].flags & ROPE_UPDATED){continue;}
		msgRopeUpdate(-1, i, &ropeList[i]);
		ropeList[i].flags |= ROPE_UPDATED;
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
