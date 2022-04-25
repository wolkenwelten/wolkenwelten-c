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

#include "entity.h"
#include "character/character.h"
#include "light.h"
#include "../main.h"
#include "../sfx/sfx.h"
#include "../gfx/frustum.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/shadow.h"
#include "../gfx/sky.h"
#include "../gfx/texture.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>
#include "../gfx/gl.h"

#define ENTITY_FADEOUT (256.f)

static void entityDraw(const entity *e){
	if((e->mesh == NULL) || (e->flags & ENTITY_HIDDEN)){return;}
	const float scale = 0.8f;
	shaderColorSimple(sMesh, lightAtPos(e->pos));

	matMov      (matMVP,matView);
	matMulTrans (matMVP,e->pos.x,e->pos.y,e->pos.z);
	matMulRotYX (matMVP,e->rot.yaw,e->rot.pitch);
	matMulScale (matMVP,scale,scale,scale);
	matMul      (matMVP,matMVP,matProjection);

	shaderMatrix(sMesh,matMVP);
	meshDraw(e->mesh);
	shadowAdd(e->pos,scale);
}

void entityDrawAll(){
	shaderBind(sMesh);
	for(uint i=0;i<entityMax;i++){
		if(entityList[i].nextFree != NULL)                        { continue; }
		if(entityDistance(&entityList[i], player) > ENTITY_FADEOUT){ continue; }
		if(!CubeInFrustum(vecSubS(entityList[i].pos,.5f),1.f))    { continue; }
		entityDraw(&entityList[i]);
	}
	shaderColor(sMesh,1.f,1.f,1.f,1.f);
}

vec entityScreenPos(const entity *e){
	const float scale = 0.8;
	matMov      (matMVP,matView);
	matMulTrans (matMVP,e->pos.x,e->pos.y,e->pos.z);
	matMulScale (matMVP,scale,scale,scale);
	matMul      (matMVP,matMVP,matProjection);
	return matMulVec(matMVP,vecNew(0,scale * 1.5,0));
}

void entityUpdateFromServer(const packet *p){
	const netEntityUpdate *data = &p->v.entityUpdate;
	const u32 ID  = data->index;
	entity *ent   = &entityList[ID];
	if(ent->generation == data->generation){return;}
	entityMax     = MAX(ID+1, entityMax);
	ent->pos      = data->pos;
	ent->rot      = data->rot;
	ent->vel      = data->vel;
	ent->flags    = data->flags;
	ent->weight   = data->weight;
	ent->mesh     = meshPear;
	ent->nextFree = NULL;
}

void entityDeleteFromServer(const packet *p){
	const netEntityDelete *data = &p->v.entityDelete;
	const u32 ID    = data->index;
	entity *ent     = &entityList[ID];
	ent->mesh       = NULL;
	ent->nextFree   = (void *)1;
	ent->generation = data->generation;
}
