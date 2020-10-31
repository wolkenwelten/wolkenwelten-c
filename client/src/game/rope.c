#include "rope.h"

#include "../game/entity.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../../../common/src/game/being.h"

mesh *ropeMesh = NULL;
#include <stdio.h>

static void ropeDrawSegment(const rope *r, const vec h, const vec p){
	(void)r;

	/*
	const float mlen = 0.f;//characterGetMaxHookLen(ghk->parent);
	if(mlen > 128.f){
		m->tex = tSteelrope;
	}else{
		m->tex = tRope;
	}*/
	// ToDo: add all rope textures to a single image, switching by X offset
	const float rlen = ropeLength(r);

	meshAddVert(ropeMesh,p.x-.05f,p.y,p.z,0.f, .5f);
	meshAddVert(ropeMesh,p.x+.05f,p.y,p.z,1.f, .5f);
	meshAddVert(ropeMesh,h.x+.05f,h.y,h.z,1.f,rlen+.5f);

	meshAddVert(ropeMesh,h.x+.05f,h.y,h.z,1.f,rlen+.5f);
	meshAddVert(ropeMesh,h.x-.05f,h.y,h.z,0.f,rlen+.5f);
	meshAddVert(ropeMesh,p.x-.05f,p.y,p.z,0.f, .5f);

	meshAddVert(ropeMesh,p.x-.05f,p.y,p.z,0.f, .5f);
	meshAddVert(ropeMesh,h.x+.05f,h.y,h.z,1.f,rlen+.5f);
	meshAddVert(ropeMesh,p.x+.05f,p.y,p.z,1.f, .5f);

	meshAddVert(ropeMesh,h.x+.05f,h.y,h.z,1.f,rlen+.5f);
	meshAddVert(ropeMesh,p.x-.05f,p.y,p.z,0.f, .5f);
	meshAddVert(ropeMesh,h.x-.05f,h.y,h.z,0.f,rlen+.5f);


	meshAddVert(ropeMesh,p.x,p.y-.05f,p.z,0.f, 0.f);
	meshAddVert(ropeMesh,p.x,p.y+.05f,p.z,1.f, 0.f);
	meshAddVert(ropeMesh,h.x,h.y+.05f,h.z,1.f,rlen);

	meshAddVert(ropeMesh,h.x,h.y+.05f,h.z,1.f,rlen);
	meshAddVert(ropeMesh,h.x,h.y-.05f,h.z,0.f,rlen);
	meshAddVert(ropeMesh,p.x,p.y-.05f,p.z,0.f, 0.f);

	meshAddVert(ropeMesh,p.x,p.y-.05f,p.z,0.f, 0.f);
	meshAddVert(ropeMesh,h.x,h.y+.05f,h.z,1.f,rlen);
	meshAddVert(ropeMesh,p.x,p.y+.05f,p.z,1.f, 0.f);

	meshAddVert(ropeMesh,h.x,h.y+.05f,h.z,1.f,rlen);
	meshAddVert(ropeMesh,p.x,p.y-.05f,p.z,0.f, 0.f);
	meshAddVert(ropeMesh,h.x,h.y-.05f,h.z,0.f,rlen);


	meshAddVert(ropeMesh,p.x,p.y,p.z-.05f,0.f, 0.f);
	meshAddVert(ropeMesh,p.x,p.y,p.z+.05f,1.f, 0.f);
	meshAddVert(ropeMesh,h.x,h.y,h.z+.05f,1.f,rlen);

	meshAddVert(ropeMesh,h.x,h.y,h.z+.05f,1.f,rlen);
	meshAddVert(ropeMesh,h.x,h.y,h.z-.05f,0.f,rlen);
	meshAddVert(ropeMesh,p.x,p.y,p.z-.05f,0.f, 0.f);

	meshAddVert(ropeMesh,p.x,p.y,p.z-.05f,0.f, 0.f);
	meshAddVert(ropeMesh,h.x,h.y,h.z+.05f,1.f,rlen);
	meshAddVert(ropeMesh,p.x,p.y,p.z+.05f,1.f, 0.f);

	meshAddVert(ropeMesh,h.x,h.y,h.z+.05f,1.f,rlen);
	meshAddVert(ropeMesh,p.x,p.y,p.z-.05f,0.f, 0.f);
	meshAddVert(ropeMesh,h.x,h.y,h.z-.05f,0.f,rlen);
}

static void ropeDraw(rope *r){
	const vec h = beingGetPos(r->a);
	const vec p = beingGetPos(r->b);

	ropeDrawSegment(r,h,r->nodes[0]->pos);
	for(int i=0;i<15;i++){
		ropeDrawSegment(r,r->nodes[i]->pos,r->nodes[i+1]->pos);
	}
	ropeDrawSegment(r,r->nodes[15]->pos,p);
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
