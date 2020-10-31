#include "../game/entity.h"

#include "../main.h"
#include "../sdl/sfx.h"
#include "../game/character.h"
#include "../gfx/frustum.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/shadow.h"
#include "../gfx/texture.h"
#include "../tmp/assets.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>
#include "../gfx/gl.h"

#define ENTITY_FADEOUT (256.f)

void entityDraw(const entity *e){
	if(e->eMesh == NULL){return;}

	matMov      (matMVP,matView);
	matMulTrans (matMVP,e->pos.x,e->pos.y+e->yoff,e->pos.z);
	matMulScale (matMVP,0.4f,0.4f,0.4f);
	matMulRotYX (matMVP,e->rot.yaw,e->rot.pitch);
	matMul      (matMVP,matMVP,matProjection);

	shaderMatrix(sShadow,matMVP);
	meshDraw(e->eMesh);
	shadowAdd(e->pos,0.5f);
}

void entityDrawAll(){
	shaderBind(sMesh);
	for(uint i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL)                         { continue; }
		if(entityDistance(&entityList[i],player) > ENTITY_FADEOUT) { continue; }
		if(!CubeInFrustum(vecSubS(entityList[i].pos,.5f),1.f))     { continue; }
		entityDraw(&entityList[i]);
	}
}
