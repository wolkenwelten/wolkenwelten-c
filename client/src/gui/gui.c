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
#include "gui.h"
#include "../main.h"
#include "../binding/widget.h"
#include "../misc/lisp.h"
#include "../misc/options.h"
#include "../game/animal.h"
#include "../game/character/character.h"
#include "../game/character/draw.h"
#include "../game/character/hook.h"
#include "../game/character/network.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../game/weather/weather.h"
#include "../sdl/sdl.h"
#include "../gfx/boundaries.h"
#include "../gfx/blockMesh.h"
#include "../gfx/frustum.h"
#include "../gfx/gl.h"
#include "../gfx/mesh.h"
#include "../gfx/gfx.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"
#include "../gfx/texture.h"
#include "../gfx/textMesh.h"
#include "../gfx/sky.h"
#include "../gui/chat.h"
#include "../gui/menu.h"
#include "../gui/menu/attribution.h"
#include "../gui/menu/inventory.h"
#include "../gui/menu/mainmenu.h"
#include "../gui/menu/multiplayer.h"
#include "../gui/menu/options.h"
#include "../gui/menu/singleplayer.h"
#include "../gui/repl.h"
#include "../gui/overlay.h"
#include "../gui/textInput.h"
#include "../gui/widgets/widgets.h"
#include "../misc/options.h"
#include "../network/chat.h"
#include "../network/client.h"
#include "../sdl/sdl.h"
#include "../tmp/objs.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../voxel/meshgen/shared.h"
#include "../../../common/src/game/chunkOverlay.h"
#include "../../../common/src/game/hook.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/game/time.h"
#include "../../../common/src/misc/colors.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/cto.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

textMesh *guim;
textMesh *crosshairMesh;
textMesh *cursorMesh;
textMesh *logoMesh;

widget *rootMenu;

widget *widgetGameScreen;

bool mouseHidden = false;
int  mousex,mousey;
uint mouseClicked[3] = {0,0,0};
uint animalOverlaysDrawn = 0;

float matOrthoProj[16];

void handlerRoot(widget *wid){
	(void)wid;
	if((widgetFocused != NULL) && (widgetFocused->type == wGameScreen)){return;}
	if(gameRunning){return;}
	openMainMenu();
	lispPanelClose();
}

void handlerRootHud(widget *wid){
	(void)wid;
	widgetFocus(widgetGameScreen);
	chatClose();
	widgetSlideH(lispPanel, 0);
}

void showMouseCursor(){
	if(!mouseHidden){return;}
	setRelativeMouseMode(mouseHidden = false);
	warpMouse(mousex,mousey);
}

void hideMouseCursor(){
	if(mouseHidden){return;}
	setRelativeMouseMode(mouseHidden = true);
	warpMouse(screenWidth/2,screenHeight/2);
}

void closeAllMenus(){
	if(gameRunning){
		menuBackground->flags |=  WIDGET_HIDDEN;
	}else{
		menuBackground->flags &= ~WIDGET_HIDDEN;
	}
	menuAttribution->flags |= WIDGET_HIDDEN;
	closeMainMenu();
	closeSingleplayerMenu();
	closeMultiplayerMenu();
	closeOptionsMenu();
	closeInventoryPanel();
	widgetFocus(NULL);
}

int getTilesize(){
	if(screenWidth < 1024){
		return 48;
	}else if(screenWidth < 1536){
		return 64;
	}else if(screenWidth < 2048){
		return 80;
	}else{
		return 96;
	}
}

void drawCrosshair(){
	if(!mouseHidden){return;}
	textMeshEmpty(crosshairMesh);
	int  off = (int)player->inaccuracy;
	int size = 16;

	textMeshAddVert(crosshairMesh,(screenWidth/2)     ,(screenHeight/2)     +off, 64.f, 64.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-size,(screenHeight/2)+size+off,  0.f,128.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+size,(screenHeight/2)+size+off,128.f,128.f,~1);

	textMeshAddVert(crosshairMesh,(screenWidth/2)     -off,(screenHeight/2)     , 64.f, 64.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-size-off,(screenHeight/2)-size,  0.f,  0.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-size-off,(screenHeight/2)+size,  0.f,128.f,~1);

	textMeshAddVert(crosshairMesh,(screenWidth/2)     ,(screenHeight/2)     -off, 64.f, 64.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+size,(screenHeight/2)-size-off,128.f,128.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)-size,(screenHeight/2)-size-off,  0.f,128.f,~1);

	textMeshAddVert(crosshairMesh,(screenWidth/2)     +off,(screenHeight/2)     , 64.f, 64.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+size+off,(screenHeight/2)+size,128.f,128.f,~1);
	textMeshAddVert(crosshairMesh,(screenWidth/2)+size+off,(screenHeight/2)-size,128.f,  0.f,~1);

	textMeshDraw(crosshairMesh);
}

void resizeUI(){
	matOrtho(matOrthoProj,0.f,screenWidth,screenHeight,0.f,-1.f,16.f);

	const int sx = 10*getTilesize();
	chatPanel->area.w = screenWidth - sx;
	chatText->area.w  = screenWidth - sx - 64;
	lispPanel->area.w = screenWidth - 128;
	if(lispPanelVisible){
		widgetSlideH(lispPanel, screenHeight-128);
	}

	initInventory();
}

static void handlerGameFocus(widget *wid){
	(void)wid;
	openInventoryPanel();
}

static void initGameOverlay(){
	cursorMesh           = textMeshNew(8);
	cursorMesh->tex      = tCursor;

	logoMesh             = textMeshNew(8);
	logoMesh->tex        = tWolkenwelten;

	crosshairMesh        = textMeshNew(16);
	crosshairMesh->tex   = tCrosshair;
	crosshairMesh->usage = GL_STREAM_DRAW;

	guim                 = textMeshNew(1<<16);
	guim->tex            = tGui;

	widgetGameScreen = widgetNewCP(wGameScreen,rootMenu,rect(0,0,-1,-1));
	widgetBind(widgetGameScreen,"focus",handlerGameFocus);
	chatPanel = widgetNewCP(wPanel,rootMenu,rect(0,-1,512,0));
	chatPanel->flags |= WIDGET_HIDDEN;
	widgetExport(widgetGameScreen, "w-game-screen");

	lispInputInit();
	chatInit();

	resizeUI();
}

void initGUI(){
	rootMenu = widgetNewCP(wSpace,NULL,rect(0,0,-1,-1));
	widgetExport(rootMenu,"w-root-menu");

	menuBackground = widgetNewCP(wSpace,rootMenu,rect(0,0,-1,-1));

	initAttributions();
	initGameOverlay();
	initMainMenu();
	initSingleplayerMenu();
	initMultiplayerMenu();
	initOptionsMenu();
}

void drawCursor(){
	const int x = mousex, y = mousey;
	if(mouseHidden){return;}

	textMeshEmpty(cursorMesh);
	textMeshAddVert(cursorMesh,x   ,y   ,  0.f,  0.f,~1);
	textMeshAddVert(cursorMesh,x   ,y+32,  0.f,128.f,~1);
	textMeshAddVert(cursorMesh,x+32,y+32,128.f,128.f,~1);

	textMeshAddVert(cursorMesh,x+32,y+32,128.f,128.f,~1);
	textMeshAddVert(cursorMesh,x+32,y   ,128.f,  0.f,~1);
	textMeshAddVert(cursorMesh,x   ,y   ,  0.f,  0.f,~1);
	textMeshDraw   (cursorMesh);
}

void updateMouse(){
	const int oldmx = mousex, oldmy = mousey;
	int nmx,nmy, btn = getMouseState(&nmx,&nmy);;
	mousex = nmx;
	mousey = nmy;

	if(mouseHidden && (widgetFocused == NULL || ((widgetFocused != NULL) && (widgetFocused->type != wGameScreen)))){
		if((mousex != oldmx) || (mousey != oldmy) || (btn != 0)){
			mouseHidden = false;
		}else{
			return;
		}
	}
	drawCursor();

	for(int cbtn = 0;cbtn < 3;cbtn++){
		if(!(btn & (1<<cbtn))){
			mouseClicked[cbtn] = 0;
		}else if((btn & (1<<cbtn)) && (getTicks() > (mouseClicked[cbtn]+500))){
			if(mouseClicked[cbtn] == 0){
				mouseClicked[cbtn] = getTicks();
			}else{
				mouseClicked[cbtn]+=50;
			}
		}
	}
}

static void drawSingleHealthbar(int hp, int maxhp, uint x, uint y, int tilesize,bool drawHeartbeat){
	const int ticks = -(getTicks() >> 4);
	int tilesizeoff,lastsize,lastoff;

	tilesizeoff = tilesize+tilesize/4;
	lastsize    = tilesize+(tilesize/2);
	lastoff     = tilesize/4;

	int heartBeat = ticks & 0x7F;
	int hbRGBA = 0xFFFFFF | (heartBeat << 25);
	int hbOff  = 16-(heartBeat>>3);
	for(int i=0;i<(maxhp>>2);i++){
		if(hp == ((i+1)*4)){
			if(drawHeartbeat){
				textMeshBox(guim,x-hbOff,y-lastoff-hbOff,lastsize+hbOff*2,lastsize+hbOff*2,31.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,hbRGBA);
			}
			textMeshBox(guim,x,y-lastoff,lastsize,lastsize,31.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
			x += tilesizeoff + lastoff;
		}else if(hp >= ((i+1)*4)){
			textMeshBox(guim,x,y,tilesize,tilesize,31.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
			x += tilesizeoff;
		}else if(((hp-1)/4) == i){
			if(drawHeartbeat){
				textMeshBox(guim,x-hbOff,y-lastoff-hbOff,lastsize+hbOff*2,lastsize+hbOff*2,31.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,hbRGBA);
			}
			textMeshBox(guim,x,y-lastoff,lastsize,lastsize,(28+((hp-1)%4))/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
			x += tilesizeoff + lastoff;
		}else{
			textMeshBox(guim,x,y,tilesize,tilesize,27.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
			x += tilesizeoff;
		}
	}
}

void drawHealthbar(){
	const int tilesize = getTilesize()/2;
	drawSingleHealthbar(player->hp, 20, tilesize/2, tilesize/2, tilesize,true);
}

void drawPlayerOverlay(uint i){
	const character *c = characterGetPlayer(i);
	if(c == NULL){return;}
	float mvp[16];
	characterCalcMVP(c, mvp);
	vec p = matMulVec(mvp,vecNew(0,0.5f,0));
	if(p.z < 0){return;}
	p.x = ((p.x / p.z)+1.f)/2.f * screenWidth;
	p.y = (1.f-((p.y / p.z)+1.f)/2.f) * screenHeight;

	u32 ofgc = guim->fgc;
	float a = MIN(255.f,MAX(0.f,(p.z-32.f)));
	guim->fgc = 0x00B0D0 | ((u32)a<<24);
	textMeshPrintfPS(guim,p.x,p.y - 16,2,"|");
	textMeshPrintfPS(guim,p.x,p.y + 16,2,"|");
	textMeshPrintfPS(guim,p.x-16,p.y,2,"-");
	textMeshPrintfPS(guim,p.x+16,p.y,2,"-");
	guim->fgc = 0xFF00C0E0;
	textMeshPrintfPS(guim,p.x+16,p.y-16,1,"%s",characterGetPlayerName(i));
	drawSingleHealthbar(characterGetPlayerHP(i),20,p.x+16,p.y-4,8,false);

	guim->fgc = ofgc;
}

const char *colorSignalHigh(int err, int warn, int good, int v){
	if(v <= err) {return ansiFG[ 9];}
	if(v <= warn){return ansiFG[11];}
	if(v >= good){return ansiFG[10];}
	return ansiFG[15];
}
const char *colorSignalLow(int err, int warn, int good, int v){
	if(v >= err) {return ansiFG[ 9];}
	if(v >= warn){return ansiFG[11];}
	if(v <= good){return ansiFG[10];}
	return ansiFG[15];
}

void drawAnimalDebugOverlay(const animal *e, int i){
	if(e == NULL)   {return;}
	if(e->type == 0){return;}
	vec p = e->screenPos;
	if(p.z <   0)         {return;}
	if(p.z > 512)         {return;}
	p.x =      ((p.x / p.z)+1.f)/2.f  * screenWidth;
	if(p.x < 0)           {return;}
	if(p.x > screenWidth) {return;}
	p.y = (1.f-((p.y / p.z)+1.f)/2.f) * screenHeight;
	if(p.y < 0)           {return;}
	if(p.y > screenHeight){return;}

	u32 ofgc = guim->fgc;
	u32 a = (u32)(MIN(128.f,MAX(0.f,(p.z-32.f)))) << 24;
	const float u = 1/32.f * (float)(32-e->type);
	const float v = 1/32.f * 30;
	textMeshBox(guim,p.x-16,p.y-16,32,32,u,v,1/32.f,1/32.f,a|0xFFFFFF);

	if(p.z > 96){guim->fgc = ofgc; return;}
	if(++animalOverlaysDrawn > 64){return;}
	guim->fgc = 0xFFFFFFFF;
	textMeshPrintfPS(guim,p.x+16,p.y-40,2,"%s",animalGetStateName(e));
	drawSingleHealthbar(e->health, animalGetMaxHealth(e), p.x+16,p.y-16,8,false);
	if(e->flags & ANIMAL_MALE){
		textMeshPrintfPS(guim,p.x+56,p.y-22,2,"\x0B");
	}else{
		textMeshPrintfPS(guim,p.x+56,p.y-22,2,"\x0C");
	}
	textMeshPrintfPS(guim,p.x-48,p.y-40,2,"%i",i);
	textMeshPrintfPS(guim,p.x-48,p.y-24,2,"%.1f",p.z);
	guim->fgc = colorPalette[7];

	const char *hungerC = colorSignalHigh(16,32,48,e->hunger);
	const char *sleepyC = colorSignalHigh(16,32,48,e->sleepy);
	const char *ageC    = colorSignalLow (78,64,48,e->age);
	textMeshPrintfPS(guim,p.x+16,p.y   ,1,"Hunger: %s%i%s",hungerC,e->hunger,ansiFG[7]);
	textMeshPrintfPS(guim,p.x+16,p.y+ 8,1,"Sleepy: %s%i%s",sleepyC,e->sleepy,ansiFG[7]);
	textMeshPrintfPS(guim,p.x+16,p.y+16,1,"Age:    %s%i%s",ageC,   e->age   ,ansiFG[7]);
	if(!(e->flags & ANIMAL_MALE)){
		textMeshPrintfPS(guim,p.x+16,p.y+24,1,"Pregn.: %i",e->pregnancy);
	}
	guim->fgc = ofgc;
}

void itemDropDrawNumbers(){
	if(player == NULL){return;}
	for(uint i=0;i<itemDropCount;i++){
		if(itemDropList[i].ent == NULL){continue;}
		if(!pointInFrustum(itemDropList[i].ent->pos)){continue;}
		const vec dist = vecSub(itemDropList[i].ent->pos, player->pos);
		const float dd = vecDot(dist,dist);
		if(dd > 9*8){continue;}
		vec p = entityScreenPos(itemDropList[i].ent);
		p.x =      ((p.x / p.z)+1.f)/2.f  * screenWidth;
		if(p.x < 0)           {continue;}
		if(p.x > screenWidth) {continue;}
		p.y = (1.f-((p.y / p.z)+1.f)/2.f) * screenHeight;
		if(p.y < 0)           {continue;}
		if(p.y > screenHeight){continue;}
		guim->sx = p.x;
		guim->sy = p.y;
		guim->size = 2;
		const u8 alpha = MIN(1, 8 - p.z) * 255;
		if(alpha == 0){continue;}
		guim->fgc = 0xFFFFFF | (alpha << 24);
		textMeshPrintfAlignCenter(guim,"%ux %s",itemDropList[i].itm.amount, itemGetName(&itemDropList[i].itm));
	}
}


static void drawHookIndicator(){
	const float hookdist = characterCanHookHit(player);
	if(hookdist < 0.f){return;}
	textMeshItemSprite(guim,screenWidth/2+8,screenHeight/2+8,32,I_Hook);
	textMeshPrintfPS(guim,screenWidth/2 + 40, screenHeight/2 + 16,2,"%.1f",hookdist);
}

void drawDebuginfo(){
	static uint ticks = 0;
	size_t tris = vboTrisCount, draws = drawCallCount;
	if(!gameRunning){return;}

	guim->font = 1;
	guim->size = 2;
	guim->sy   = screenHeight/2+32;
	if(recvBytesCurrentSession <= 0){
		guim->sx   = screenWidth/2-(10*16);
		textMeshPrintf(guim,"%.*s",20 + ((ticks++ >> 4)&3),"Connecting to server...");
	}else if(player->flags & CHAR_SPAWNING){
		guim->sx   = screenWidth/2-(8*16);
		textMeshPrintf(guim,"%.*s",13 + ((ticks++ >> 4)&3),"Respawning...");
	}else if(!playerChunkActive){
		guim->sx   = screenWidth/2-(8*16);
		textMeshPrintf(guim,"%.*s",13 + ((ticks++ >> 4)&3),"Loading World...");
	}else if(goodbyeSent){
		guim->sx   = screenWidth/2-(8*16);
		textMeshPrintf(guim,"%.*s",13 + ((ticks++ >> 4)&3),"Closing . . .");
	}
	guim->size = 2;
	guim->sx   = screenWidth;
	guim->sy   = 4;
	textMeshPrintfAlignRight(guim,"%s",VERSION);
	drawHookIndicator();

	if(player->flags & CHAR_CONS_MODE){
		guim->sx   = screenWidth  - 256;
		guim->sy   = screenHeight - 280;
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"CONS Mode");
	}

	guim->sx   = screenWidth-112;
	guim->sy   = 32;
	textMeshPrintf(guim,"FPS %s%3.0f\n",colorSignalHigh(screenRefreshRate/3-1,screenRefreshRate/2-1,screenRefreshRate-1,curFPS),curFPS);
	guim->fgc  = colorPalette[15];

	guim->sy  += 16;
	guim->sx   = screenWidth;
	guim->size = 2;
	for(uint i=0;i<32;i++){
		const char *cname = characterGetPlayerName(i);
		if(i == (uint)playerID){continue;}
		if(cname == NULL){continue;}
		guim->sx = screenWidth;
		guim->sy = 64+(i*42);
		textMeshPrintfAlignRight(guim,"%s",cname);
		drawSingleHealthbar(characterGetPlayerHP(i),20,screenWidth-96,guim->sy+22,14,false);
		drawPlayerOverlay(i);
	}

	const u64 curPing = getTicks();
	if(curPing > lastPing + 30000){
		guim->sx   = (screenWidth/2);
		guim->sy   = (screenHeight/2)-(screenHeight/8);
		guim->size = 4;
		guim->fgc  = colorPalette[9];
		textMeshPrintfAlignCenter(guim,"Critically High Ping!!!");
		guim->fgc  = colorPalette[15];
	}else if(curPing > lastPing + 3000){
		guim->sx   = (screenWidth/2);
		guim->sy   = (screenHeight/2)-(screenHeight/4);
		guim->size = 2;
		guim->fgc  = colorPalette[11];
		textMeshPrintfAlignCenter(guim,"High Ping!");
		guim->fgc  = colorPalette[15];
	}

	vboTrisCount = 0;
	drawCallCount = 0;
	if(optionDebugInfo){
		guim->font = 1;
		guim->sx   = 4;
		guim->sy   = getTilesize();
		guim->size = 4;
		textMeshPrintf(guim,"%s\n",gtimeGetTimeOfDayHRS(gtimeGetTimeOfDay()));
		guim->size = 1;
		textMeshPrintf(guim,"Active Tris.: %s%s\n",colorSignalLow(1<<21,1<<19,1<<18,tris),getHumanReadableSize(tris));
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Draw calls  : %s%s\n",colorSignalLow(1<<15,1<<14,1<<8,draws),getHumanReadableSize(draws));
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Particles   : %s%s\n",colorSignalLow(1<<16,1<<15,1<<14,particleCount),getHumanReadableSize(particleCount));
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Sparticles  : %s%s\n",colorSignalLow(1<<15,1<<14,1<<13,sparticleCount),getHumanReadableSize(sparticleCount));
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"RainDrops   : %s%s\n",colorSignalLow(1<<16,1<<15,1<<14,     rainCount),getHumanReadableSize(rainCount));
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"SnowFlakes  : %s%s\n",colorSignalLow(1<<16,1<<15,1<<14,     snowCount),getHumanReadableSize(snowCount));
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Chunks gener: %i\n",chunkGetGeneratedThisFrame());
		textMeshPrintf(guim,"Snow Power  : %i\n",snowIntensity);
		textMeshPrintf(guim,"Storm Power : %i [%i]\n",stormIntensity, stormDelta);
		textMeshPrintf(guim,"ChunkVert   : %uK\n",blockMeshUsedBytes()/1024);
		textMeshPrintf(guim,"ActiveChungi: %i\n",chungusGetActiveCount());
		textMeshPrintf(guim,"ChnkOverlays: %u [Free:%u]\n", chunkOverlayAllocated, chunkOverlayAllocated - chunkOverlayUsed);
		textMeshPrintf(guim,"GarbageRuns : %u\n",lGCRuns);
		textMeshPrintf(guim,"Latency     : %s%u\n",colorSignalLow(400,200,50,lastLatency),lastLatency);
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"WorstFrame  : %s%u\n",colorSignalLow(60,20,18,worstFrame),worstFrame);
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Brightness  : %f\n",gtimeGetBrightness(gtimeGetTimeOfDay()));
		textMeshPrintf(guim,"Cloudyness  : %i\n",player->cloudyness);
		textMeshPrintf(guim,"DirtyChunks : %u\n",chunksDirtied);
		textMeshPrintf(guim,"ChunksCopied: %u\n",chunksCopied);

		animalOverlaysDrawn = 0;
		for(uint i=0;i<animalListMax;i++){
			drawAnimalDebugOverlay(&animalList[i],i);
		}
	}
	itemDropDrawNumbers();
	guim->font = 0;
}

void drawAmmunition(){
	if(player == NULL){return;}
	item *activeItem = &player->inventory[player->activeItem];
	if(activeItem == NULL){return;}
	if(itemIsEmpty(activeItem)){return;}
	int ammo = itemGetAmmunition(activeItem);
	if(ammo <= 0){return;}
	int amount = characterGetItemAmount(player,ammo);

	const int tilesize = getTilesize();

	guim->sx = screenWidth-(tilesize*2.f);
	guim->sy = screenHeight-tilesize-inventoryPanel->area.h;
	guim->font = 1;
	textMeshNumber(guim,guim->sx,guim->sy,2,amount);
	textMeshItemSprite(guim,guim->sx+32,guim->sy-18,64,ammo);

	if(itemGetStackSize(activeItem) <= 1){
		const int magSize = itemGetMagazineSize(activeItem);
		if(magSize){
			guim->sx += 4;
			textMeshNumber(guim,guim->sx-32,guim->sy-tilesize+tilesize/3,2,itemGetAmmo(activeItem));
			textMeshNumber(guim,guim->sx+32,guim->sy-tilesize+tilesize/3,2,magSize);
			textMeshDigit(guim,guim->sx-12,guim->sy-tilesize+tilesize/3, 2, 10);
		}
	}
	guim->font = 0;
}

void drawMenuBackground(){
	static uint ticks = 0;
	hsvaColor hsv;
	hsv.h = 160 + sinf(++ticks*0.0002f) * 16.f;
	hsv.s = 128;
	hsv.v = 128 + cosf(++ticks*0.0009f) * 28.f;

	if((skyBrightness < 0.6f) && (skyBrightness > 0.5f)){
		const float bright = 1.f - fabsf((MAX(0.f,(skyBrightness - 0.5f)) * 20.f) - 1.f);
		hsv.h = (((int)(bright * 140.f)) + 160) & 0xFF;
	}

	u32 ccolor = RGBAToU(hsvToRGB(hsv));
	glClearColor( (ccolor&0xFF)/256.f, ((ccolor>>8)&0xFF)/256.f, ((ccolor>>16)&0xFF)/256.f, 1.f );
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	textMeshEmpty(logoMesh);
	int off = -128;
	int size = MIN(screenWidth,screenHeight)/4;

	textMeshAddVert(logoMesh,off+(screenWidth/2)-size,(screenHeight/2)-size,  0.f,  0.f,~1);
	textMeshAddVert(logoMesh,off+(screenWidth/2)+size,(screenHeight/2)+size,128.f,128.f,~1);
	textMeshAddVert(logoMesh,off+(screenWidth/2)+size,(screenHeight/2)-size,128.f,  0.f,~1);

	textMeshAddVert(logoMesh,off+(screenWidth/2)+size,(screenHeight/2)+size,128.f,128.f,~1);
	textMeshAddVert(logoMesh,off+(screenWidth/2)-size,(screenHeight/2)-size,  0.f,  0.f,~1);
	textMeshAddVert(logoMesh,off+(screenWidth/2)-size,(screenHeight/2)+size,  0.f,128.f,~1);

	textMeshDraw(logoMesh);
}

void drawHud(){
	textMeshEmpty(guim);
	guim->wrap = 0;
	guim->font = 1;
	guim->size = 2;

	if(gameRunning){
		drawOverlay(guim);
		drawHealthbar();
		drawDebuginfo();
		drawAmmunition();
		chatDraw(guim);
	}
	lispPanelCheckAutoComplete();
	lispPanelCheckAutoCompleteDescription();
	const box2D screen = rect(0,0,screenWidth,screenHeight);
	widget *hover = widgetDraw(rootMenu,guim,screen);
	widgetDoMouseEvents(rootMenu,hover, screen);
	if(isInventoryOpen()){drawInventory(guim);}
	widgetDrawPopups(guim);
	textMeshDraw(guim);
}

void renderUI(){
	gfxGroupStart("UI");

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	shaderBind(sTextMesh);
	shaderMatrix(sTextMesh,matOrthoProj);
	if(widgetFocused && (widgetFocused->type == wGameScreen)){
		hideMouseCursor();
	}else{
		showMouseCursor();
	}
	drawHud();
	drawCrosshair();
	updateMouse();

	glEnable(GL_DEPTH_TEST);
	gfxGroupEnd();
}

bool guiCancel(){
	if(!gameRunning){return true;}
	if(lispPanelVisible){
		lispPanelClose();
		return true;
	}
	if(isInventoryOpen()){
		closeInventory();
		return true;
	}
	if(widgetFocused == chatText){
		handlerRootHud(NULL);
		return true;
	}
	closeAllMenus();
	widgetFocus(widgetGameScreen);
	return false;
}

void guiEscape(){
	if(guiCancel()){return;}
	if(widgetFocused == widgetGameScreen){
		openMainMenu();
		return;
	}
}
