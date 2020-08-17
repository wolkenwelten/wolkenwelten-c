#include "gui.h"


#include "../main.h"
#include "../misc/options.h"
#include "../game/blockType.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/item.h"
#include "../game/itemDrop.h"
#include "../mods/mods.h"
#include "../gui/menu.h"
#include "../sdl/sdl.h"
#include "../gfx/glew.h"
#include "../gfx/mesh.h"
#include "../gfx/objs.h"
#include "../gfx/gfx.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"
#include "../gfx/texture.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../tmp/cto.h"
#include "../gui/inventory.h"
#include "../gui/overlay.h"
#include "../gui/textInput.h"
#include "../network/chat.h"
#include "../network/client.h"
#include "../../../common/src/misc/misc.h"

#include <string.h>
#include <stdio.h>

#define ITEMTILE (1.f/32.f)

textMesh *guim;
textMesh *crosshairMesh;
textMesh *cursorMesh;

bool mouseHidden = false;
int mousex,mousey;
int mouseClicked[3] = {0,0,0};

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

void drawCrosshair(){
	textMeshEmpty(crosshairMesh);
	int  off = (int)player->inaccuracy;
	int size = 16;
	if(off > 64){off=64;}

	textMeshAddVert(crosshairMesh,(screenWidth/2)     ,(screenHeight/2)     +off, 64.f, 64.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+size,(screenHeight/2)+size+off,128.f,128.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-size,(screenHeight/2)+size+off,  0.f,128.f,~1);

	textMeshAddVert(crosshairMesh,(screenWidth/2)     -off,(screenHeight/2)     , 64.f, 64.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-size-off,(screenHeight/2)-size,  0.f,  0.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-size-off,(screenHeight/2)+size,  0.f,128.f,~1);

	textMeshAddVert(crosshairMesh,(screenWidth/2)     ,(screenHeight/2)     -off, 64.f, 64.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-size,(screenHeight/2)-size-off,  0.f,128.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+size,(screenHeight/2)-size-off,128.f,128.f,~1);

	textMeshAddVert(crosshairMesh,(screenWidth/2)     +off,(screenHeight/2)     , 64.f, 64.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+size+off,(screenHeight/2)+size,128.f,128.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+size+off,(screenHeight/2)-size,128.f,  0.f,~1);

	textMeshDraw(crosshairMesh);
}

void resizeUI(){
	matOrtho(matOrthoProj,0.f,(float)screenWidth,(float)screenHeight,0.f,-1.f,10.f);
}

void initUI(){
	cursorMesh           = textMeshNew();
	cursorMesh->tex      = tCursor;

	crosshairMesh        = textMeshNew();
	crosshairMesh->tex   = tCrosshair;
	crosshairMesh->usage = GL_STREAM_DRAW;

	guim                 = textMeshNew();
	guim->tex            = tGui;

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
	textMeshDraw   (cursorMesh);
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

	for(int cbtn = 0;cbtn < 3;cbtn++){
		if(!(btn & (1<<cbtn))){
			if(mouseClicked[cbtn]){
				updateMenuClick(mousex,mousey,cbtn+1);
			}
			mouseClicked[cbtn] = 0;
		}else if((btn & (1<<cbtn)) && ((int)getTicks() > (mouseClicked[cbtn]+500))){
			if(mouseClicked[cbtn] == 0){
				mouseClicked[cbtn] = getTicks();
			}else{
				mouseClicked[cbtn]+=50;
			}
			updateInventoryClick(mousex,mousey,cbtn+1);
		}
	}
}

void drawDebuginfo(){
	static unsigned int ticks=0;
	int tris = vboTrisCount;

	if(recvBytesCurrentSession <= 0){
		guim->sx   = screenWidth/2-(10*16);
		guim->sy   = screenHeight/2+32;
		guim->size = 2;
		textMeshPrintf(guim,"%.*s",20 + ((ticks++ >> 4)&3),"Connecting to server...");
	}else if(!playerChunkActive){
		guim->sx   = screenWidth/2-(8*16);
		guim->sy   = screenHeight/2+32;
		guim->size = 2;
		textMeshPrintf(guim,"%.*s",13 + ((ticks++ >> 4)&3),"Loading World...");
	}

	guim->sx   = 4;
	guim->sy   = screenHeight-40;
	guim->size = 2;

	textMeshPrintf(guim,"FPS %.0f\n",curFPS);
	textMeshPrintf(guim,"Ver. %s [%.8s]",VERSION,COMMIT);

	vboTrisCount = 0;
	if(!optionDebugInfo){return;}

	guim->sx   =  4;
	guim->sy   = 76;
	guim->size =  1;

	textMeshPrintf(guim,"Player     X: %05.2f VX: %02.4f\n",player->x,player->vx);
	textMeshPrintf(guim,"Player     Y: %05.2f VY: %02.4f\n",player->y,player->vy);
	textMeshPrintf(guim,"Player     Z: %05.2f VZ: %02.4f\n",player->z,player->vz);
	textMeshPrintf(guim,"Player   Yaw: %04.2f\n",player->yaw);
	textMeshPrintf(guim,"Player Pitch: %04.2f\n",player->pitch);
	textMeshPrintf(guim,"Player  Roll: %04.2f\n",player->roll);
	textMeshPrintf(guim,"Player  Hoff: %04.2f\n",player->hitOff);
	textMeshPrintf(guim,"Player Shake: %04.2f\n",player->shake);
	textMeshPrintf(guim,"Active Tris.: %s\n", getHumanReadableSize(tris));
	textMeshPrintf(guim,"Active Part.: %s\n", getHumanReadableSize(particleCount));
	textMeshPrintf(guim,"Player Layer: %2i\n",((int)player->y/CHUNGUS_SIZE));
	textMeshPrintf(guim,"Entities    : %2i\n",entityCount);
	textMeshPrintf(guim,"Chunks gener: %2i\n",chunkGetGeneratedThisFrame());
	textMeshPrintf(guim,"ActiveChunks: %s\n",getHumanReadableSize(chunkGetActive()));
	textMeshPrintf(guim,"FreeChunks  : %2i\n",chunkGetFree());
	textMeshPrintf(guim,"ActiveChungi: %2i\n",chungusGetActiveCount());
	textMeshPrintf(guim,"Bytes Sent  : %sB\n",getHumanReadableSize(sentBytesCurrentSession));
	textMeshPrintf(guim,"Bytes Recvd : %sB\n",getHumanReadableSize(recvBytesCurrentSession));
	textMeshPrintf(guim,"Uncompressed: %sB\n",getHumanReadableSize(recvUncompressedBytesCurrentSession));
	textMeshPrintf(guim,"Comp. Ratio : %2.2fX\n",(float)recvUncompressedBytesCurrentSession / (float)recvBytesCurrentSession);
	textMeshPrintf(guim,"Canvas Size : %ix%i\n",screenWidth,screenHeight);
	textMeshPrintf(guim,"Itemdrops   : %i\n",itemDropCount);
}

void drawItemBar(){
	int playerSelection = player->activeItem;
	int tilesize;
	if(screenWidth < 1024){
		tilesize = 48;
	}else if(screenWidth < 1536){
		tilesize = 64;
	}else {
		tilesize = 80;
	}

	guim->size = 2;
	for(int i = 0;i<10;i++){
		int x = (screenWidth-10*tilesize)+(i*tilesize);
		int y = screenHeight-tilesize;
		int style = 0;
		if(i == playerSelection){
			style = 1;
		}
		textMeshItem(guim,x,y,tilesize,style,&player->inventory[i]);
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
	lastsize    = tilesize+(tilesize/2);
	lastoff     = tilesize/4;
	x = y = tilesize/2;

	int heartBeat = --ticks & 0x7F;
	int hbRGBA = 0xFFFFFF | (heartBeat << 25);
	int hbOff  = 16-(heartBeat>>3);
	ticks = ticks & 0xFF;
	for(int i=0;i<5;i++){
		if(player->hp == ((i+1)*4)){
			textMeshBox(guim,x-hbOff,y-lastoff-hbOff,lastsize+hbOff*2,lastsize+hbOff*2,31.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,hbRGBA);
			textMeshBox(guim,x,y-lastoff,lastsize,lastsize,31.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
			x += tilesizeoff + lastoff;
		}else if(player->hp >= ((i+1)*4)){
			textMeshBox(guim,x,y,tilesize,tilesize,31.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
			x += tilesizeoff;
		}else if(((player->hp-1)/4) == i){
			textMeshBox(guim,x-hbOff,y-lastoff-hbOff,lastsize+hbOff*2,lastsize+hbOff*2,31.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,hbRGBA);
			textMeshBox(guim,x,y-lastoff,lastsize,lastsize,(28+((player->hp-1)%4))/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
			x += tilesizeoff + lastoff;
		}else{
			textMeshBox(guim,x,y,tilesize,tilesize,27.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
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
	const float ix =  1.3f;
	const float iy = -0.9f;
	const float iz = -1.5f;

	shaderBind(sMesh);
	if(itemHasPrimaryAction(activeItem)){
		matTranslation(matViewAI,ix,animOff+iy,iz + hitOff*0.3f);
		matMulRotYX(matViewAI,hitOff*10.f,hitOff*45.f);
		matMul(matViewAI, matViewAI, matProjection);
		shaderMatrix(sMesh, matViewAI);
	}else{
		float y = iy+animOff-(hitOff/8);
		matTranslation(matViewAI,(0.5f+ix)-hitOff*1.2f,y+(hitOff/3),iz - hitOff*1.1f);
		matMulRotYX(matViewAI,hitOff*10.f,hitOff*-35.f);
		matMul(matViewAI, matViewAI, matProjection);
		shaderMatrix(sMesh, matViewAI);
	}
	meshDraw(aiMesh);
}


void drawAmmunition(){
	item *activeItem = &player->inventory[player->activeItem];
	if(activeItem == NULL){return;}
	if(itemIsEmpty(activeItem)){return;}
	int ammo = getAmmunitionDispatch(activeItem);
	if(ammo <= 0){return;}
	int amount = characterGetItemAmount(player,ammo);

	int tilesize;
	if(screenWidth < 1024){
		tilesize = 48;
	}else if(screenWidth < 1536){
		tilesize = 64;
	}else {
		tilesize = 80;
	}

	guim->sx = screenWidth-(tilesize*12)+(tilesize*0.3f);
	guim->sy = screenHeight-tilesize+(tilesize*0.3f);
	textMeshNumber(guim,guim->sx,guim->sy,2,amount);

	int u = ammo % 32;
	int v = ammo / 32;
	textMeshBox(guim,guim->sx+32,guim->sy-18,64,64,u*ITEMTILE,v*ITEMTILE,1.f/32.f,1.f/32.f,~1);
}

void drawChat(){
	guim->sy   = screenHeight - (13*16);
	guim->sx   = 4;
	guim->size = 1;
	for(int i=0;i<8;i++){
		textMeshAddString(guim,chatLog[i]);
		guim->sy += 8;
	}
	guim->size = 2;
	for(int i=8;i<12;i++){
		textMeshAddString(guim,chatLog[i]);
		guim->sy += 16;
	}
}

void drawHud(){
	textMeshEmpty(guim);

	drawOverlay(guim);
	drawHealthbar();
	drawDebuginfo();
	drawAmmunition();
	if(isInventoryOpen()){
		drawInventory(guim);
	}else{
		drawItemBar();
	}
	drawChat();
	textMeshDraw(guim);
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
		drawCrosshair();
	}
	textInputDraw();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}
