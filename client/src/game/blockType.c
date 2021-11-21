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

#include "../game/blockType.h"

#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/sky.h"
#include "../../../common/src/misc/misc.h"

void blockTypeGenMeshes(){
	for(int i=0;i<256;i++){
		if(!blockTypeValid(i)){continue;}
		blocks[i].singleBlock = meshNew(NULL);
		mesh *singleBlock = blocks[i].singleBlock;
		singleBlock->dataCount = 0;
		singleBlock->tex = tBlocks;
		blockTypeAddToMesh(i,singleBlock,vecNew(-0.5f,-0.5f,-0.5f),vecOne());
		meshFinishStatic(singleBlock);
	}
}

void blockTypeSetTex(blockId b, side cside, uint tex){
	blocks[b].tex[cside]  = tex;
	blocks[b].texX[cside] = tex & 0xF;
	blocks[b].texY[cside] = tex >> 4;
}

void blockTypeAddToMesh(blockId b, mesh *m, const vec pos, const vec size) {
	float tileLoX[6];
	float tileHiX[6];
	float tileLoY[6];
	float tileHiY[6];
	const float TILE = (1.0f/16.f);
	const float x    = pos.x;
	const float y    = pos.y;
	const float z    = pos.z;
	const float w    = size.x;
	const float h    = size.y;
	const float d    = size.z;

	for(int cside=0;cside<6;cside++){
		tileLoX[cside] = ((float)blocks[b].texX[cside]*TILE);
		tileLoY[cside] = ((float)blocks[b].texY[cside]*TILE);
		tileHiX[cside] = tileLoX[cside] + TILE;
		tileHiY[cside] = tileLoY[cside] + TILE;
	}

	// Front Face
	meshAddVertC(m, x  ,y  ,z+d,tileLoX[0],tileHiY[0],0.75f);
	meshAddVertC(m, x+w,y  ,z+d,tileHiX[0],tileHiY[0],0.75f);
	meshAddVertC(m, x+w,y+h,z+d,tileHiX[0],tileLoY[0],0.75f);
	meshAddVertC(m, x+w,y+h,z+d,tileHiX[0],tileLoY[0],0.75f);

	meshAddVertC(m, x  ,y+h,z+d,tileLoX[0],tileLoY[0],0.75f);
	meshAddVertC(m, x  ,y  ,z+d,tileLoX[0],tileHiY[0],0.75f);

	// Back Face
	meshAddVertC(m, x  ,y  ,z  ,tileHiX[1],tileHiY[1],0.75f);
	meshAddVertC(m, x  ,y+h,z  ,tileHiX[1],tileLoY[1],0.75f);
	meshAddVertC(m, x+w,y+h,z  ,tileLoX[1],tileLoY[1],0.75f);
	meshAddVertC(m, x+w,y+h,z  ,tileLoX[1],tileLoY[1],0.75f);
	meshAddVertC(m, x+w,y  ,z  ,tileLoX[1],tileHiY[1],0.75f);
	meshAddVertC(m, x  ,y  ,z  ,tileHiX[1],tileHiY[1],0.75f);

	// Top Face
	meshAddVertC(m, x  ,y+h,z  ,tileLoX[2],tileLoY[2],1.f);
	meshAddVertC(m, x  ,y+h,z+d,tileLoX[2],tileHiY[2],1.f);
	meshAddVertC(m, x+w,y+h,z+d,tileHiX[2],tileHiY[2],1.f);
	meshAddVertC(m, x+w,y+h,z+d,tileHiX[2],tileHiY[2],1.f);
	meshAddVertC(m, x+w,y+h,z  ,tileHiX[2],tileLoY[2],1.f);
	meshAddVertC(m, x  ,y+h,z  ,tileLoX[2],tileLoY[2],1.f);

	// Bottom Face
	meshAddVertC(m, x  ,y  ,z  ,tileHiX[3],tileLoY[3],0.5f);
	meshAddVertC(m, x+w,y  ,z  ,tileLoX[3],tileLoY[3],0.5f);
	meshAddVertC(m, x+w,y  ,z+d,tileLoX[3],tileHiY[3],0.5f);
	meshAddVertC(m, x+w,y  ,z+d,tileLoX[3],tileHiY[3],0.5f);
	meshAddVertC(m, x  ,y  ,z+d,tileHiX[3],tileHiY[3],0.5f);
	meshAddVertC(m, x  ,y  ,z  ,tileHiX[3],tileLoY[3],0.5f);

	// Right face
	meshAddVertC(m, x+w,y  ,z  ,tileHiX[4],tileHiY[4],0.75f);
	meshAddVertC(m, x+w,y+h,z  ,tileHiX[4],tileLoY[4],0.75f);
	meshAddVertC(m, x+w,y+h,z+d,tileLoX[4],tileLoY[4],0.75f);
	meshAddVertC(m, x+w,y+h,z+d,tileLoX[4],tileLoY[4],0.75f);
	meshAddVertC(m, x+w,y  ,z+d,tileLoX[4],tileHiY[4],0.75f);
	meshAddVertC(m, x+w,y  ,z  ,tileHiX[4],tileHiY[4],0.75f);

	// Left Face
	meshAddVertC(m, x  ,y  ,z  ,tileLoX[5],tileHiY[5],0.75f);
	meshAddVertC(m, x  ,y  ,z+d,tileHiX[5],tileHiY[5],0.75f);
	meshAddVertC(m, x  ,y+h,z+d,tileHiX[5],tileLoY[5],0.75f);
	meshAddVertC(m, x  ,y+h,z+d,tileHiX[5],tileLoY[5],0.75f);
	meshAddVertC(m, x  ,y+h,z  ,tileLoX[5],tileLoY[5],0.75f);
	meshAddVertC(m, x  ,y  ,z  ,tileLoX[5],tileHiY[5],0.75f);
}

void blockTypeDraw(blockId b, vec pos, float alpha, int depthOffset){
	matMov      (matMVP,matView);
	matMulTrans (matMVP,pos.x,pos.y,pos.z);
	matMul      (matMVP,matMVP,matProjection);

	if(depthOffset){
		glPolygonOffset(depthOffset,depthOffset);
		glEnable(GL_POLYGON_OFFSET_FILL);
	}

	shaderBind(sMesh);
	shaderMatrix(sMesh,matMVP);
	shaderColor(sMesh, worldBrightness, worldBrightness, worldBrightness, alpha);
	meshDraw(blocks[b].singleBlock);
	shaderColor(sMesh, worldBrightness, worldBrightness, worldBrightness, 1.f);

	if(depthOffset){
		glPolygonOffset(0,0);
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}
