#include "rope.h"

#include "../game/entity.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../../../common/src/game/being.h"

mesh *ropeMesh = NULL;

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

static void ropeDraw(rope *r){
	const vec h = beingGetPos(r->a);
	const vec p = beingGetPos(r->b);
	ropeDrawSegment(r,h,p);
}

void ropeInit(){
	ropeMesh = meshNew();
	ropeMesh->tex = tRope;
}

void ropeDrawAll(){
	if(ropeCount == 0){return;}
	shaderBind(sMesh);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshEmpty(ropeMesh);
	for(uint i=0;i<ropeCount;i++){
		if(beingType(ropeList[i].a) == 0){continue;}
		ropeDraw(&ropeList[i]);
	}
	meshFinishStream(ropeMesh);
	meshDraw(ropeMesh);
}
