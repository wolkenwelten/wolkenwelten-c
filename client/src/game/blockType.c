#include "../game/blockType.h"

#include "../gfx/texture.h"
#include "../misc/misc.h"
#include "../gfx/glew.h"

typedef struct {
	unsigned char texX[6];
	unsigned char texY[6];
	unsigned int color[2];
	mesh singleBlock;
	int hp;
	blockCategory cat;
	char *name;
} blockType;

blockType blocks[256];


void blockTypeGenMeshes(){
	for(int i=0;i<256;i++){
		if(!blockTypeValid(i)){continue;}
		mesh *singleBlock = &blocks[i].singleBlock;
		singleBlock->dataCount = 0;
		singleBlock->tex = tBlocks;
		blockTypeAddToMesh(i,singleBlock,-0.5f,-0.5f,-0.5f,1.f,1.f,1.f);
		meshFinish(singleBlock, GL_STATIC_DRAW);
	}
}

void blockTypeInitBlock(uint8_t b, uint32_t tex, blockCategory ncat,const char *bname,int nhp,unsigned int ncolor1,unsigned int ncolor2){
	for(int i=0;i<6;i++){
		blockTypeSetTex(b,i,tex);
	}
	blocks[b].name = (char *)bname;
	blocks[b].hp = nhp;
	blocks[b].cat = ncat;
	blocks[b].color[0] = ncolor1;
	blocks[b].color[1] = ncolor2;
}

const char *blockTypeGetName(uint8_t b){
	return blocks[b].name;
}

int blockTypeGetHP(uint8_t b){
	return blocks[b].hp;
}
blockCategory blockTypeGetCat(uint8_t b){
	return blocks[b].cat;
}

uint16_t blockTypeGetTexX(uint8_t b, int side){
	return blocks[b].texX[side];
}

uint16_t blockTypeGetTexY(uint8_t b, int side){
	return blocks[b].texY[side];
}

mesh *blockTypeGetMesh(uint8_t b){
	return &blocks[b].singleBlock;
}

uint32_t blockTypeGetParticleColor(unsigned char b) {
	return blocks[b].color[rngValR()&1];
}

bool blockTypeValid(uint8_t b){
	return blocks[b].name != NULL;
}

void blockTypeInit(){
	blockTypeInitBlock( 1, 1, DIRT,  "Dirt",           200, 0xFF0A234F,0xFF051B45);
	blockTypeInitBlock( 2, 7, DIRT,  "Grass",          240, 0xFF004227,0xFF051B45); blockTypeSetTex(2,2,0); blockTypeSetTex(2,3,1);
	blockTypeInitBlock( 3, 2, STONE, "Stone",         1000, 0xFF5E5E5E,0xFF484848);
	blockTypeInitBlock( 4, 3, STONE, "Coal",           800, 0xFF262626,0xFF101010);
	blockTypeInitBlock( 5, 4, WOOD,  "Spruce Log",     500, 0xFF051B25,0xFF07161D);
	blockTypeInitBlock( 6, 5, LEAVES,"Spruce Leaves",   40, 0xFF012C12,0xFF01250F);
	blockTypeInitBlock( 7, 6, WOOD,  "Roots",          170, 0xFF14323E,0xFF0D2029);
	blockTypeInitBlock( 8, 6, WOOD,  "Dirt Roots",     480, 0xFF14323E,0xFF0D2029);
	blockTypeInitBlock( 9, 8, STONE, "Obsidian",      2000, 0xFF222222,0xFF171717);
	blockTypeInitBlock(10, 9, WOOD,  "Oak Log",        500, 0xFF082C3C,0xFF08242E);
	blockTypeInitBlock(11,10, LEAVES,"Oak Leaves",      40, 0xFF004227,0xFF003318);
	blockTypeInitBlock(12,12, STONE, "Marble Block",  1600, 0xFFF0F0F0,0xFFEBEBEB);
	blockTypeInitBlock(13,11, STONE, "Hematite Ore",  1100, 0xFF5B5B72,0xFF5E5E5E);
	blockTypeInitBlock(14,13, STONE, "Marble Pillar", 1600, 0xFFF0F0F0,0xFFEBEBEB); blockTypeSetTex(14,2,12); blockTypeSetTex(14,3,12);
	blockTypeInitBlock(15,14, STONE, "Marble Blocks", 1600, 0xFFF0F0F0,0xFFEBEBEB);
	blockTypeInitBlock(16,16, WOOD,  "Hewn Log",       400, 0xFF09678f,0xFF1380af); blockTypeSetTex(16,2,15); blockTypeSetTex(16,3,15);
	blockTypeInitBlock(17,17, WOOD,  "Boards",         300, 0xFF09678f,0xFF1380af);

	blockTypeGenMeshes();
}

void blockTypeSetTex(uint8_t b, int side, uint32_t tex){
	blocks[b].texX[side] = tex & 0xF;
	blocks[b].texY[side] = tex >> 4;
}

void blockTypeAddToMesh(uint8_t b, mesh *m, float x, float y, float z, float w, float h, float d) {
	float tileLoX[6];
	float tileHiX[6];
	float tileLoY[6];
	float tileHiY[6];
	const float TILE = (1.0f/16.f);

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
