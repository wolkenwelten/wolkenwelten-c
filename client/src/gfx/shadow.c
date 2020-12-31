#include "shadow.h"

#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../tmp/assets.h"
#include "../voxel/bigchungus.h"

#include <math.h>
#include "../gfx/gl.h"

mesh *meshShadow = NULL;

void shadowInit(){
	meshShadow = meshNew();
	meshShadow->tex = textureNew(gfx_shadow_png_data,gfx_shadow_png_len,"client/gfx/shadow.png");
}

void shadowDraw(){
	glDepthFunc      (GL_LEQUAL);
	meshFinishStream (meshShadow);
	shaderBind       (sShadow);
	matMov           (matMVP,matView);
	matMul           (matMVP,matMVP,matProjection);
	shaderMatrix     (sShadow,matMVP);

	glPolygonOffset(-16,-16);
	glEnable(GL_POLYGON_OFFSET_FILL);

	meshDraw         (meshShadow);

	glPolygonOffset(0,0);
	glDisable(GL_POLYGON_OFFSET_FILL);
	meshEmpty(meshShadow);
}

void shadowAdd(const vec pos, float size){
	vec p = pos;
	p.y = floorf(p.y);
	for(int i=0;i<12;i++){
		if(worldGetB(p.x,p.y-1.f,p.z) != 0){break;}
		p.y -= 1.f;
		if(i == 7){return;}
	}
	const float s = (1.2f + ((pos.y - p.y)/4.f))*size;
	const float a = MAX(0.f,1.f-((pos.y - p.y)/11.f));
	p.x -= s/2.f;
	p.z -= s/2.f;

	meshAddVertC(meshShadow, p.x  ,p.y,p.z  ,0.f,0.f,a);
	meshAddVertC(meshShadow, p.x+s,p.y,p.z+s,1.f,1.f,a);
	meshAddVertC(meshShadow, p.x+s,p.y,p.z  ,1.f,0.f,a);

	meshAddVertC(meshShadow, p.x+s,p.y,p.z+s,1.f,1.f,a);
	meshAddVertC(meshShadow, p.x  ,p.y,p.z  ,0.f,0.f,a);
	meshAddVertC(meshShadow, p.x  ,p.y,p.z+s,0.f,1.f,a);
}
