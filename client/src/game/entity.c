#include "../game/entity.h"

#include "../main.h"
#include "../sdl/sfx.h"
#include "../game/character.h"
#include "../gfx/frustum.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../tmp/assets.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>
#include "../gfx/gl.h"

entity  entityList[1<<14];
int     entityCount = 0;
entity *entityFirstFree = NULL;
mesh   *meshShadow = NULL;

#define ENTITY_FADEOUT (128.f)

void entityInit(){
	meshShadow = meshNew();
	meshShadow->tex = textureNew(gfx_shadow_png_data,gfx_shadow_png_len,"client/gfx/shadow.png");
}

entity *entityNew(vec pos, vec rot){
	entity *e = NULL;
	if(entityFirstFree == NULL){
		e = &entityList[entityCount++];
	}else{
		e = entityFirstFree;
		entityFirstFree = e->nextFree;
		if(entityFirstFree == e){
			entityFirstFree = NULL;
		}
	}
	entityReset(e);

	e->pos = pos;
	e->rot = rot;

	return e;
}

void entityFree(entity *e){
	if(e == NULL){return;}
	e->nextFree = entityFirstFree;
	entityFirstFree = e;
	if(e->nextFree == NULL){
		e->nextFree = e;
	}
}

void entityShadowDraw(){
	glDepthFunc(GL_LEQUAL);
	meshFinish  (meshShadow, GL_STREAM_DRAW);
	matMov      (matMVP,matView);
	matMul      (matMVP,matMVP,matProjection);
	shaderMatrix(sMesh,matMVP);
	meshDraw    (meshShadow);
}

void entityShadow(const entity *e){
	vec p = e->pos;
	p.y = floorf(p.y)+0.05f;
	for(int i=0;i<12;i++){
		if(worldGetB(p.x,p.y-1.f,p.z) != 0){break;}
		p.y -= 1.f;
		if(i == 7){return;}
	}
	const float s = 0.4f + ((e->pos.y - p.y)/8.f);
	const float a = MAX(0.f,1.f-((e->pos.y - p.y)/11.f));
	p.x -= s/2.f;
	p.z -= s/2.f;

	meshAddVertC(meshShadow, p.x  ,p.y,p.z  ,0.f,0.f,a);
	meshAddVertC(meshShadow, p.x+s,p.y,p.z+s,1.f,1.f,a);
	meshAddVertC(meshShadow, p.x+s,p.y,p.z  ,1.f,0.f,a);

	meshAddVertC(meshShadow, p.x+s,p.y,p.z+s,1.f,1.f,a);
	meshAddVertC(meshShadow, p.x  ,p.y,p.z  ,0.f,0.f,a);
	meshAddVertC(meshShadow, p.x  ,p.y,p.z+s,0.f,1.f,a);
}

void entityDraw(const entity *e){
	if(e->eMesh == NULL){return;}

	matMov      (matMVP,matView);
	matMulTrans (matMVP,e->pos.x,e->pos.y+e->yoff,e->pos.z);
	matMulScale (matMVP,0.25f,0.25f,0.25f);
	matMulRotYX (matMVP,e->rot.yaw,e->rot.pitch);
	matMul      (matMVP,matMVP,matProjection);

	shaderBind(sShadow);
	shaderMatrix(sShadow,matMVP);
	meshDraw(e->eMesh);
	entityShadow(e);
}

void entityDrawAll(){
	shaderBind(sMesh);
	if(meshShadow == NULL){entityInit();}
	meshEmpty(meshShadow);
	for(int i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(entityDistance(&entityList[i],player) > (ENTITY_FADEOUT * ENTITY_FADEOUT)){ continue; }
		if(!CubeInFrustum(vecSubS(entityList[i].pos,.5f),1.f)){continue;}
		entityDraw(&entityList[i]);
	}
	entityShadowDraw();
}

void entityUpdateAll(){
	for(int i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(!(entityList[i].flags & ENTITY_UPDATED)){
			entityUpdate(&entityList[i]);
		}
		entityList[i].flags &= ~ENTITY_UPDATED;
	}
}
