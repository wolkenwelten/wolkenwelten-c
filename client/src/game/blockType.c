#include "../game/blockType.h"

#include "../gfx/gl.h"
#include "../gfx/mesh.h"
#include "../gfx/texture.h"
#include "../../../common/src/misc/misc.h"

void blockTypeGenMeshes(){
	for(int i=0;i<256;i++){
		if(!blockTypeValid(i)){continue;}
		blocks[i].singleBlock = meshNew();
		mesh *singleBlock = blocks[i].singleBlock;
		singleBlock->dataCount = 0;
		singleBlock->tex = tBlocks;
		blockTypeAddToMesh(i,singleBlock,vecNew(-0.5f,-0.5f,-0.5f),vecOne());
		meshFinish(singleBlock, GL_STATIC_DRAW);
	}
}

void blockTypeSetTex(uint8_t b, int side, uint32_t tex){
	blocks[b].texX[side] = tex & 0xF;
	blocks[b].texY[side] = tex >> 4;
}

void blockTypeAddToMesh(u8 b, mesh *m, const vec pos, const vec size) {
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

	for(int side=0;side<6;side++){
		tileLoX[side] = ((float)blocks[b].texX[side]*TILE);
		tileLoY[side] = ((float)blocks[b].texY[side]*TILE);
		tileHiX[side] = tileLoX[side] + TILE;
		tileHiY[side] = tileLoY[side] + TILE;
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
	meshAddVertC(m, x  ,y  ,z  ,tileHiX[3],tileLoY[3],.5f);
	meshAddVertC(m, x+w,y  ,z  ,tileLoX[3],tileLoY[3],.5f);
	meshAddVertC(m, x+w,y  ,z+d,tileLoX[3],tileHiY[3],.5f);
	meshAddVertC(m, x+w,y  ,z+d,tileLoX[3],tileHiY[3],.5f);
	meshAddVertC(m, x  ,y  ,z+d,tileHiX[3],tileHiY[3],.5f);
	meshAddVertC(m, x  ,y  ,z  ,tileHiX[3],tileLoY[3],.5f);

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
