#include "gui.h"


#include "../main.h"
#include "../misc/options.h"
#include "../game/blockType.h"
#include "../game/entity.h"
#include "../game/item.h"
#include "../sdl/sdl.h"
#include "../gfx/glew.h"
#include "../gfx/mesh.h"
#include "../gfx/objs.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"
#include "../gfx/texture.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../gui/inventory.h"
#include "../tmp/cto.h"
#include "../gui/textInput.h"
#include "../network/chat.h"
#include "../network/client.h"

#include <string.h>

#define ITEMTILE (1.f/32.f)

textMesh *guim;
textMesh *textm;
textMesh *itemMesh;
textMesh *crosshairMesh;
textMesh *cursorMesh;


bool mouseLClicked = false;
bool mouseRClicked = false;
bool mouseHidden   = false;
int mousex,mousey;

float matOrthoProj[16];

void showMouseCursor(){
	setRelativeMouseMode(false);
	warpMouse(mousex,mousey);
	mouseHidden = false;
}

void hideMouseCursor(){
	setRelativeMouseMode(true);
	mouseHidden = true;
}

void initCrosshair(){
	textMeshEmpty(crosshairMesh);

	textMeshAddVert(crosshairMesh,(screenWidth/2)-16,(screenHeight/2)-16,  0.f,  0.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-16,(screenHeight/2)+16,  0.f,128.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+16,(screenHeight/2)+16,128.f,128.f,~1);

	textMeshAddVert(crosshairMesh,(screenWidth/2)+16,(screenHeight/2)+16,128.f,128.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+16,(screenHeight/2)-16,128.f,  0.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-16,(screenHeight/2)-16,  0.f,  0.f,~1);
}

void resizeUI(){
	initCrosshair();
	matOrtho(matOrthoProj,0.f,(float)screenWidth,(float)screenHeight,0.f,-1.f,10.f);
}

void initUI(){
	itemMesh             = textMeshNew();
	cursorMesh           = textMeshNew();
	crosshairMesh        = textMeshNew();
	guim                 = textMeshNew();
	textm                = textMeshNew();

	itemMesh->tex        = tItems;
	cursorMesh->tex      = tCursor;
	guim->tex            = tGui;
	textm->tex           = tGui;
	crosshairMesh->tex   = tCrosshair;

	crosshairMesh->usage = GL_STATIC_DRAW;

	resizeUI();
}

void drawCursor(){
	const int x = mousex;
	const int y = mousey;
	if(mouseHidden){return;}

	textMeshEmpty(cursorMesh);
	textMeshAddVert(cursorMesh,   x,   y,  0.f,  0.f,~1);
	textMeshAddVert(cursorMesh,   x,y+32,  0.f,128.f,~1);
	textMeshAddVert(cursorMesh,x+32,y+32,128.f,128.f,~1);

	textMeshAddVert(cursorMesh,x+32,y+32,128.f,128.f,~1);
	textMeshAddVert(cursorMesh,x+32,   y,128.f,  0.f,~1);
	textMeshAddVert(cursorMesh,   x,   y,  0.f,  0.f,~1);
	textMeshDraw(cursorMesh);
}

void updateMouse(){
	const int oldmx = mousex;
	const int oldmy = mousey;
	int btn = getMouseState(&mousex,&mousey);

	if(mouseHidden){
		if((mousex != oldmx) || (mousey != oldmy) || (btn != 0)){
			mouseHidden = false;
		}
	}
	drawCursor();

	if((btn & 1) && !mouseLClicked){
		mouseLClicked = true;
		if(isInventoryOpen()){
			updateInventoryClick(mousex,mousey,1);
		}
	}else if(mouseLClicked){
		mouseLClicked = false;
	}
	if((btn & 4) && !mouseRClicked){
		mouseRClicked = true;
		if(isInventoryOpen()){
			updateInventoryClick(mousex,mousey,3);
		}
	}else if(mouseRClicked){
		mouseRClicked = false;
	}
}

const char *getHumanReadableSize(size_t n){
	static char buf[32];
	const char *suffix[] = {"B","KB","MB","GB","TB"};
	int i;

	for(i=0;i<5;i++){
		if(n<1024){break;}
		n = n >> 10;
	}
	i = snprintf(buf,sizeof(buf),"%llu%s",(long long unsigned int)n,suffix[i]);
	buf[sizeof(buf)-1] = 0;
	return buf;
}

void drawDebuginfo(){
	static unsigned int ticks=0;
	int tris = vboTrisCount;

	if(recvBytesCurrentSession <= 0){
		textm->sx   = screenWidth/2-(10*16);
		textm->sy   = screenHeight/2+32;
		textm->size = 2;
		textMeshPrintf(textm,"%.*s",20 + ((ticks++ >> 4)&3),"Connecting to server...");
	}else if(!playerChunkActive){
		textm->sx   = screenWidth/2-(8*16);
		textm->sy   = screenHeight/2+32;
		textm->size = 2;
		textMeshPrintf(textm,"%.*s",13 + ((ticks++ >> 4)&3),"Loading World...");
	}

	textm->sx   = 4;
	textm->sy   = screenHeight-40;
	textm->size = 2;

	textMeshPrintf(textm,"FPS %.0f\n",curFPS);
	textMeshPrintf(textm,"Ver. %s [%.8s]",VERSION,COMMIT);

	vboTrisCount = 0;
	if(!optionDebugInfo){return;}

	textm->sx   =  4;
	textm->sy   = 76;
	textm->size =  1;

	textMeshPrintf(textm,"Player     X: %05.2f VX: %02.4f\n",player->x,player->vx);
	textMeshPrintf(textm,"Player     Y: %05.2f VY: %02.4f\n",player->y,player->vy);
	textMeshPrintf(textm,"Player     Z: %05.2f VZ: %02.4f\n",player->z,player->vz);
	textMeshPrintf(textm,"Player   Yaw: %04.2f\n",player->yaw);
	textMeshPrintf(textm,"Player Pitch: %04.2f\n",player->pitch);
	textMeshPrintf(textm,"Player  Roll: %04.2f\n",player->roll);
	textMeshPrintf(textm,"Player  Hoff: %04.2f\n",player->hitOff);
	textMeshPrintf(textm,"Player Shake: %04.2f\n",player->shake);
	textMeshPrintf(textm,"Active Tris.: %i\n", tris);
	textMeshPrintf(textm,"Player Layer: %2i\n",((int)player->y/CHUNGUS_SIZE));
	textMeshPrintf(textm,"Entities    : %2i\n",entityCount);
	textMeshPrintf(textm,"Chunks gener: %2i\n",chunkGetGeneratedThisFrame());
	textMeshPrintf(textm,"ActiveChunks: %2i\n",chunkGetActive());
	textMeshPrintf(textm,"FreeChunks  : %2i\n",chunkGetFree());
	textMeshPrintf(textm,"ActiveChungi: %2i\n",chungusGetActiveCount());
	textMeshPrintf(textm,"Bytes Sent  : %s\n",getHumanReadableSize(sentBytesCurrentSession));
	textMeshPrintf(textm,"Bytes Recvd : %s\n",getHumanReadableSize(recvBytesCurrentSession));
}

void drawItemBar(){
	item *cItem;
	unsigned char a;
	unsigned short b;
	int playerSelection = player->activeItem;
	int tilesize,itemtilesize,itemtilesizeoff;
	if(screenWidth < 1024){
		tilesize = 48;
		itemtilesize = 32;
	}else if(screenWidth < 1536){
		tilesize = 64;
		itemtilesize = 48;
	}else {
		tilesize = 80;
		itemtilesize = 64;
	}
	itemtilesizeoff = (tilesize-itemtilesize)/2;

	textm->size = 2;
	for(int i = 0;i<10;i++){
		int x = (screenWidth-10*tilesize)+(i*tilesize);
		int y = screenHeight-tilesize;
		if(i == playerSelection){
			textMeshBox(guim,x,y,tilesize,tilesize,5.f/8.f,1.f/8.f,1.f/8.f,1.f/8.f,~1);
		}else{
			textMeshBox(guim,x,y,tilesize,tilesize,4.f/8.f,1.f/8.f,1.f/8.f,1.f/8.f,~1);
		}
		cItem = &player->inventory[i];
		if(cItem == NULL){continue;}
		b = cItem->ID;
		a = cItem->amount;
		if(a == 0){continue;}
		int u = b % 32;
		int v = b / 32;
		textMeshBox(itemMesh,x+itemtilesizeoff,y+itemtilesizeoff,itemtilesize,itemtilesize,u*ITEMTILE,v*ITEMTILE,1.f/32.f,1.f/32.f,~1);
		if(!itemIsSingle(cItem)){
			textm->sx = x+tilesize-tilesize/4;
			textm->sy = y+(itemtilesize-itemtilesizeoff)+tilesize/32;
			textMeshNumber(textm,a);
		}
	}
}

void drawHealthbar(){
	static unsigned int ticks = 0;
	int tilesize,tilesizeoff,x,y,lastsize,lastoff;
	if(screenWidth < 1024){
		tilesize = 16;
	}else if(screenWidth < 1536){
		tilesize = 24;
	}else{
		tilesize = 32;
	}

	tilesizeoff = tilesize+tilesize/4;
	lastsize = tilesize+(tilesize/2);
	lastoff = tilesize/4;
	x = y = tilesize/2;

	int heartBeat = --ticks & 0x7F;
	int hbRGBA = 0xFFFFFF | (heartBeat << 25);
	int hbOff  = 16-(heartBeat>>3);
	ticks = ticks & 0xFF;
	for(int i=0;i<5;i++){
		if(player->hp == ((i+1)*4)){
			textMeshBox(guim,x-hbOff,y-lastoff-hbOff,lastsize+hbOff*2,lastsize+hbOff*2,7.f/8.f,0.f,1.f/8.f,1.f/8.f,hbRGBA);
			textMeshBox(guim,x,y-lastoff,lastsize,lastsize,7.f/8.f,0.f,1.f/8.f,1.f/8.f,~1);
			x += tilesizeoff + lastoff;
		}else if(player->hp >= ((i+1)*4)){
			textMeshBox(guim,x,y,tilesize,tilesize,7.f/8.f,0.f,1.f/8.f,1.f/8.f,~1);
			x += tilesizeoff;
		}else if(((player->hp-1)/4) == i){
			textMeshBox(guim,x-hbOff,y-lastoff-hbOff,lastsize+hbOff*2,lastsize+hbOff*2,7.f/8.f,0.f,1.f/8.f,1.f/8.f,hbRGBA);
			textMeshBox(guim,x,y-lastoff,lastsize,lastsize,(4+((player->hp-1)%4))/8.f,0.f,1.f/8.f,1.f/8.f,~1);
			x += tilesizeoff + lastoff;
		}else{
			textMeshBox(guim,x,y,tilesize,tilesize,7.f/8.f,1.f/8.f,1.f/8.f,1.f/8.f,~1);
			x += tilesizeoff;
		}
	}
}

void drawActiveItem(){
	float matViewAI[16];
	item *activeItem = &player->inventory[player->activeItem];
	if(activeItem == NULL){return;}
	if(itemIsEmpty(activeItem)){return;}

	mesh *aiMesh = itemGetMesh(activeItem);
	if(aiMesh == NULL){return;}

	float animOff = player->yoff;
	float hitOff  = player->hitOff;

	shaderBind(sMesh);
	if(itemHasMineAction(activeItem)){
		matTranslation(matViewAI,1.8f,-0.9f,-1.3f + hitOff*0.3f);
		matMulRotYX(matViewAI,hitOff*10.f,hitOff*45.f);
		matMul(matViewAI, matViewAI, matProjection);
		shaderMatrix(sMesh, matViewAI);

	}else{
		float y = -0.9f+animOff-(hitOff/8);
		matTranslation(matViewAI,1.8f-hitOff*1.2f,y+(hitOff/3),-1.3f - hitOff*1.1f);
		matMulRotYX(matViewAI,hitOff*10.f,hitOff*-35.f);
		matMul(matViewAI, matViewAI, matProjection);
		shaderMatrix(sMesh, matViewAI);
	}
	meshDraw(aiMesh);
}

void drawChat(){
	textm->sy   = screenHeight - (13*16);
	textm->sx   = 4;
	textm->size = 1;
	for(int i=0;i<8;i++){
		textMeshAddString(textm,chatLog[i]);
		textm->sy += 8;
	}
	textm->size = 2;
	for(int i=8;i<12;i++){
		textMeshAddString(textm,chatLog[i]);
		textm->sy += 16;
	}
}

static uint32_t     overlayColorAnimSrc      = 0;
static uint32_t     overlayColorAnimDst      = 0;
static unsigned int overlayColorAnimStart    = 0;
static unsigned int overlayColorAnimEnd      = 1;
static uint32_t     overlayColorAnimNew      = 0;
static unsigned int overlayColorAnimDur      = 0;

uint32_t colorInterpolate(uint32_t c1,uint32_t c2,float i){
	if(i < 0.f){return c1;}
	if(i > 1.f){return c2;}

	const float r1 = (c1    ) & 0xFF;
	const float r2 = (c2    ) & 0xFF;
	const float g1 = (c1>> 8) & 0xFF;
	const float g2 = (c2>> 8) & 0xFF;
	const float b1 = (c1>>16) & 0xFF;
	const float b2 = (c2>>16) & 0xFF;
	const float a1 = (c1>>24) & 0xFF;
	const float a2 = (c2>>24) & 0xFF;
	const float        i2 = 1.f-i;

	const float r = (((float)r1 * i2) + ((float)r2 * i));
	const float g = (((float)g1 * i2) + ((float)g2 * i));
	const float b = (((float)b1 * i2) + ((float)b2 * i));
	const float a = (((float)a1 * i2) + ((float)a2 * i));

	return (uint32_t)r | ((uint32_t)g<<8) | ((uint32_t)b<<16) | ((uint32_t)a << 24);
}



void commitOverlayColor(){
	if(overlayColorAnimNew == overlayColorAnimDst){return;}
	overlayColorAnimStart = getTicks();
	overlayColorAnimEnd   = overlayColorAnimStart + overlayColorAnimDur;
	overlayColorAnimSrc   = overlayColorAnimDst;
	overlayColorAnimDst   = overlayColorAnimNew;
}

uint32_t getOverlayColor(){
	int animDur = overlayColorAnimEnd - overlayColorAnimStart;
	int off =  getTicks() - overlayColorAnimStart;
	float i = ((float)off) / ((float)animDur);
	return colorInterpolate(overlayColorAnimSrc,overlayColorAnimDst,i);
}

void setOverlayColor(unsigned int color, unsigned int animationDuration){
	if(animationDuration == 0){animationDuration = 1;}
	overlayColorAnimNew = color;
	overlayColorAnimDur = animationDuration;
}

void resetOverlayColor(){
	setOverlayColor(overlayColorAnimDst&0x00FFFFFF,300);
}

void drawOverlay(){
	uint32_t c = getOverlayColor();
	if((c&0xFF000000) == 0){return;}
	textMeshBox(guim, 0, 0, screenWidth, screenHeight, 0.5f, 0.25f, 1.f/16.f, 1.f/16.f, getOverlayColor());
}

void drawHud(){
	textMeshEmpty(textm);
	textMeshEmpty(guim);
	textMeshEmpty(itemMesh);

	drawOverlay();
	drawHealthbar();
	drawDebuginfo();
	if(isInventoryOpen()){
		drawInventory(guim, textm, itemMesh);
	}else{
		drawItemBar();
	}
	drawChat();
	textMeshDraw(guim);
	textMeshDraw(itemMesh);
	textMeshDraw(textm);
}

void renderUI(){
	drawActiveItem();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	shaderBind(sTextMesh);
	shaderMatrix(sTextMesh,matOrthoProj);
	drawHud();
	if(isInventoryOpen()){
		updateMouse();
	}else{
		textMeshDraw(crosshairMesh);
	}
	textInputDraw();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}
