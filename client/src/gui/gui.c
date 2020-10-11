#include "gui.h"

#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

#include "../main.h"
#include "../misc/options.h"
#include "../game/animal.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../sdl/sdl.h"
#include "../gfx/gl.h"
#include "../gfx/mesh.h"
#include "../gfx/gfx.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"
#include "../gfx/texture.h"
#include "../gfx/textMesh.h"
#include "../gui/menu.h"
#include "../gui/inventory.h"
#include "../gui/overlay.h"
#include "../gui/textInput.h"
#include "../network/chat.h"
#include "../network/client.h"
#include "../sdl/sdl.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/tmp/cto.h"
#include "../../../common/src/mods/mods.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

#define ITEMTILE (1.f/32.f)

textMesh *guim;
textMesh *crosshairMesh;
textMesh *cursorMesh;

widget *rootHud   = NULL;
widget *chatPanel = NULL;
widget *chatText  = NULL;

bool mouseHidden = false;
uint mousex,mousey;
uint mouseClicked[3] = {0,0,0};

float matOrthoProj[16];

void handlerRootHud(widget *wid){
	(void)wid;
	chatText->vals[0] = 0;
	widgetFocus(NULL);
	widgetSlideH(chatPanel, 0);
	hideMouseCursor();
}

void showMouseCursor(){
	setRelativeMouseMode(false);
	warpMouse(mousex,mousey);
	mouseHidden = false;
}

void hideMouseCursor(){
	setRelativeMouseMode(true);
	mouseHidden = true;
	chatText->vals[0] = 0;
	widgetSlideH(chatPanel, 0);
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

	const int sx = 10*getTilesize();
	chatPanel->w = screenWidth - sx;
	chatText->w = screenWidth - sx - 64;

	initInventory();
}

void openChat(){
	if(widgetFocused != NULL){return;}
	widgetSlideH(chatPanel, 64);
	widgetFocus(chatText);
	showMouseCursor();
}

void handlerChatSubmit(widget *wid){
	if(chatText->vals[0] != 0){
		msgSendChatMessage(chatText->vals);
		chatResetHistorySel();
	}
	handlerRootHud(wid);
}

void handlerChatSelectPrev(widget *wid){
	const char *msg = chatGetPrevHistory();
	if(*msg == 0){return;}
	memcpy(wid->vals,msg,256);
	textInputFocus(wid);
}

void handlerChatSelectNext(widget *wid){
	const char *msg = chatGetNextHistory();
	if(*msg == 0){return;}
	memcpy(wid->vals,msg,256);
	textInputFocus(wid);
}

void handlerChatBlur(widget *wid){
	(void)wid;
	chatResetHistorySel();
}


void initUI(){
	cursorMesh           = textMeshNew();
	cursorMesh->tex      = tCursor;

	crosshairMesh        = textMeshNew();
	crosshairMesh->tex   = tCrosshair;
	crosshairMesh->usage = GL_STREAM_DRAW;

	guim                 = textMeshNew();
	guim->tex            = tGui;

	rootHud = widgetNewCP(wSpace,NULL,0,0,-1,-1);

	chatPanel = widgetNewCP(wPanel,rootHud,0,-1,512,0);
	chatPanel->flags |= WIDGET_HIDDEN;
	chatText  = widgetNewCPLH(wTextInput,chatPanel,16,16,440,32,"Message","submit",handlerChatSubmit);
	widgetBind(chatText,"blur",handlerChatBlur);
	widgetBind(chatText,"selectPrev",handlerChatSelectPrev);
	widgetBind(chatText,"selectNext",handlerChatSelectNext);
	widgetNewCPLH(wButton,chatPanel,-16,16,24,32,"\xA8","click",handlerChatSubmit);

	resizeUI();
}

void drawCursor(){
	const int x = mousex, y = mousey;
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
	const uint oldmx = mousex, oldmy = mousey;
	int nmx,nmy, btn = getMouseState(&nmx,&nmy);;
	mousex = nmx;
	mousey = nmy;

	if(mouseHidden){
		if((mousex != oldmx) || (mousey != oldmy) || (btn != 0)){
			mouseHidden = false;
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

static void drawSingleHealthbar(int hp, uint x, uint y, int tilesize,bool drawHeartbeat){
	const int ticks = -(getTicks() >> 4);
	int tilesizeoff,lastsize,lastoff;

	tilesizeoff = tilesize+tilesize/4;
	lastsize    = tilesize+(tilesize/2);
	lastoff     = tilesize/4;

	int heartBeat = ticks & 0x7F;
	int hbRGBA = 0xFFFFFF | (heartBeat << 25);
	int hbOff  = 16-(heartBeat>>3);
	for(int i=0;i<5;i++){
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
	drawSingleHealthbar(player->hp, tilesize/2, tilesize/2, tilesize,true);
}

void drawPlayerOverlay(uint i){
	const character *c = characterGetPlayer(i);
	if(c == NULL){return;}

	matMov(matMVP,matView);
	matMulTrans(matMVP,c->pos.x,c->pos.y+c->yoff,c->pos.z);
	matMulRotYX(matMVP,-c->rot.yaw,-c->rot.pitch/6.f);
	matMul(matMVP,matMVP,matProjection);
	vec p = matMulVec(matMVP,vecNew(0.f,0.25f,0.f));
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
	guim->fgc = 0xC000B0D0;
	textMeshPrintfPS(guim,p.x+16,p.y-16,1,"%s",characterGetPlayerName(i));
	drawSingleHealthbar(characterGetPlayerHP(i), p.x+16,p.y-4,8,false);

	guim->fgc = ofgc;
}

void drawDebuginfo(){
	static uint ticks = 0;
	int tris = vboTrisCount;

	guim->font = 1;
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
	}else if(player->flags & CHAR_SPAWNING){
		guim->sx   = screenWidth/2-(8*16);
		guim->sy   = screenHeight/2+32;
		guim->size = 2;
		textMeshPrintf(guim,"%.*s",13 + ((ticks++ >> 4)&3),"Respawning...");
	}
	guim->size = 1;
	guim->sx   = screenWidth;
	guim->sy   = 4;
	textMeshPrintfRA(guim,"%s",VERSION,COMMIT);

	guim->sx   = screenWidth-48;
	guim->sy   = 14;
	textMeshPrintf(guim,"FPS %02.0f\n",curFPS);

	guim->sy  += 16;
	guim->sx   = screenWidth;
	guim->size = 2;
	for(uint i=0;i<32;i++){
		const char *cname = characterGetPlayerName(i);
		if(cname == NULL){continue;}
		textMeshPrintfRA(guim,"%s",cname);
		drawSingleHealthbar(characterGetPlayerHP(i),screenWidth-96,guim->sy+22,14,false);
		guim->sy += 42;
		guim->sx = screenWidth;
		drawPlayerOverlay(i);
	}

	vboTrisCount = 0;
	if(!optionDebugInfo){
		guim->font = 0;
		return;
	}

	guim->sx   =  4;
	guim->sy   = getTilesize();
	guim->size =  1;
	textMeshPrintf(guim,"Player     X: %05.2f VX: %02.4f GVX: %02.4f\n",player->pos.x,player->vel.x,player->gvel.x);
	textMeshPrintf(guim,"Player     Y: %05.2f VY: %02.4f GVY: %02.4f\n",player->pos.y,player->vel.y,player->gvel.y);
	textMeshPrintf(guim,"Player     Z: %05.2f VZ: %02.4f GVZ: %02.4f\n",player->pos.z,player->vel.z,player->gvel.z);
	textMeshPrintf(guim,"Player   Yaw: %04.2f\n",player->rot.yaw);
	textMeshPrintf(guim,"Player Pitch: %04.2f\n",player->rot.pitch);
	textMeshPrintf(guim,"Player  Roll: %04.2f\n",player->rot.roll);
	textMeshPrintf(guim,"Player Flags: %08X\n",player->flags);
	textMeshPrintf(guim,"Active Tris.: %s\n", getHumanReadableSize(tris));
	textMeshPrintf(guim,"Particles   : %s\n", getHumanReadableSize(particleCount));
	textMeshPrintf(guim,"Animals     : %2i\n",animalCount);
	textMeshPrintf(guim,"Entities    : %2i\n",entityCount);
	textMeshPrintf(guim,"Itemdrops   : %i\n",itemDropCount);
	textMeshPrintf(guim,"Chunks gener: %2i\n",chunkGetGeneratedThisFrame());
	textMeshPrintf(guim,"ActiveChunks: %s\n",getHumanReadableSize(chunkGetActive()));
	textMeshPrintf(guim,"FreeChunks  : %2i\n",chunkGetFree());
	textMeshPrintf(guim,"ActiveChungi: %2i\n",chungusGetActiveCount());
	textMeshPrintf(guim,"Bytes Sent  : %sB\n",getHumanReadableSize(sentBytesCurrentSession));
	textMeshPrintf(guim,"Bytes Recvd : %sB\n",getHumanReadableSize(recvBytesCurrentSession));
	textMeshPrintf(guim,"Uncompressed: %sB\n",getHumanReadableSize(recvUncompressedBytesCurrentSession));
	textMeshPrintf(guim,"Comp. Ratio : %2.2fX\n",(float)recvUncompressedBytesCurrentSession / (float)recvBytesCurrentSession);
	textMeshPrintf(guim,"Canvas Size : %ix%i\n",screenWidth,screenHeight);

	guim->font = 0;

}

void drawActiveItem(){
	float matViewAI[16];
	item *activeItem = &player->inventory[player->activeItem];
	if(activeItem == NULL){return;}
	if(itemIsEmpty(activeItem)){return;}

	mesh *aiMesh = getMeshDispatch(activeItem);
	if(aiMesh == NULL){return;}

	const float ix =  1.6f;
	float iy = -0.9f;
	const float iz = -1.5f;
	float hitOff,y;

	if(!(player->flags & CHAR_FALLING)){
		iy += sinf((float)(player->breathing-96)/512.f)*0.05f;
	}

	shaderBind(sMesh);
	switch(player->animationIndex){
		default:
			hitOff = animationInterpolation(player->animationTicksLeft,player->animationTicksMax,0.3f);
			y = iy+player->yoff-(hitOff/8);
			matTranslation(matViewAI,ix-hitOff*1.2f,y+(hitOff/3),iz - hitOff*1.1f);
			matMulRotYX(matViewAI,hitOff*10.f,hitOff*-35.f);
		break;

		case 1:
			hitOff = animationInterpolation(player->animationTicksLeft,player->animationTicksMax,0.5f);
			matTranslation(matViewAI,ix,player->yoff+iy,iz + hitOff);
			matMulRotYX(matViewAI,hitOff*10.f,hitOff*45.f);
		break;

		case 2:
			hitOff = animationInterpolationSustain(player->animationTicksLeft,player->animationTicksMax,0.3f,0.5f);
			y = iy+player->yoff-(hitOff/8);
			matTranslation(matViewAI,ix-hitOff*0.5f,y-(hitOff*0.6f),iz - hitOff*0.4f);
			matMulRotYX(matViewAI,hitOff*15.f,hitOff*-55.f);
		break;

		case 3:
			hitOff = animationInterpolation(player->animationTicksLeft,player->animationTicksMax,0.5f);
			matTranslation(matViewAI,ix,player->yoff+iy,iz + hitOff*0.1f);
			matMulRotYX(matViewAI,hitOff*3.f,hitOff*9.f);
		break;

		case 4:
			hitOff = animationInterpolation(player->animationTicksLeft,player->animationTicksMax,1.f)*3.f;
			if(hitOff < 1.f){
				matTranslation(matViewAI,ix-hitOff*1.4,player->yoff+iy,iz + hitOff*0.3f);
				matMulRotYX(matViewAI,hitOff*20.f,hitOff*40.f);
			}else if(hitOff < 2.f){
				hitOff = hitOff-1.f;
				matTranslation(matViewAI,ix-1.4f,player->yoff+iy-hitOff*0.2f,iz + 0.3f);
				matMulRotYX(matViewAI,hitOff*60.f+20.f,hitOff*120.f+40.f);
				matMulScale(matViewAI, 1.f-hitOff, 1.f-hitOff, 1.f-hitOff);
			}else if(hitOff < 3.f){
				hitOff = hitOff-2.f;
				matTranslation(matViewAI,ix,player->yoff+iy-(1.f-hitOff)*2.f,iz);
				matMulRotYX(matViewAI,(1.f-hitOff)*3.f,(1.f-hitOff)*9.f);
			}
		break;

		case 5:
			hitOff = (float)player->animationTicksLeft / (float)player->animationTicksMax;
			y = iy+player->yoff-hitOff;
			matTranslation(matViewAI,ix-hitOff*0.5f,y-(hitOff*0.6f),iz - hitOff*0.4f);
			matMulRotYX(matViewAI,hitOff*30.f,hitOff*-70.f);
		break;
	};
	matMul(matViewAI, matViewAI, matProjection);
	shaderMatrix(sMesh, matViewAI);
	meshDraw(aiMesh);
}

void drawActiveGlider(){
	static u64 ticks = 0;
	float matViewAI[16];
	if(player->gliderFade < 0.01f){return;}

	float deg  = ((float)++ticks*0.4f);
	float yoff = cos(deg*2.1f)*player->shake;
	float xoff = sin(deg*1.3f)*player->shake;

	float breath = sinf((float)(player->breathing-256)/512.f)*3.f;
	if(player->flags & CHAR_FALLING){
		breath = 0.f;
	}

	shaderBind(sMesh);
	matTranslation(matViewAI,0.f,player->yoff+0.9f,-0.65f);
	matMulRotYX(matViewAI,0.f+xoff,player->rot.pitch*-0.08f + yoff + breath);
	matMulScale(matViewAI,player->gliderFade, player->gliderFade, player->gliderFade);
	matMul(matViewAI, matViewAI, matProjection);
	shaderMatrix(sMesh, matViewAI);
	meshDraw(meshGlider);
}


void drawAmmunition(){
	item *activeItem = &player->inventory[player->activeItem];
	if(activeItem == NULL){return;}
	if(itemIsEmpty(activeItem)){return;}
	int ammo = getAmmunitionDispatch(activeItem);
	if(ammo <= 0){return;}
	int amount = characterGetItemAmount(player,ammo);

	const int tilesize = getTilesize();

	guim->sx = screenWidth-(tilesize*2.f);
	guim->sy = screenHeight-tilesize-inventoryPanel->h;
	guim->font = 1;
	textMeshNumber(guim,guim->sx,guim->sy,2,amount);
	textMeshItemSprite(guim,guim->sx+32,guim->sy-18,64,ammo);

	if(getStackSizeDispatch(activeItem) <= 1){
		if(hasGetMagSize(activeItem)){
			guim->sx += 4;
			textMeshNumber(guim,guim->sx-32,guim->sy-tilesize+tilesize/3,2,itemGetAmmo(activeItem));
			textMeshNumber(guim,guim->sx+32,guim->sy-tilesize+tilesize/3,2,getMagSizeDispatch(activeItem));
			textMeshDigit(guim,guim->sx-12,guim->sy-tilesize+tilesize/3, 2, 10);
		}
	}
	guim->font = 0;
}

void drawChat(){
	guim->sy   = screenHeight - (9*16) - (chatPanel->h-8);
	guim->sx   = 24;
	guim->size = 1;
	guim->font = 1;
	for(int i=0;i<8;i++){
		textMeshAddString(guim,chatLog[i]);
		guim->sy += 8;
	}
	guim->size = 2;
	for(int i=8;i<12;i++){
		textMeshAddString(guim,chatLog[i]);
		guim->sy += 16;
	}
	guim->font = 0;
}

void drawHud(){
	textMeshEmpty(guim);

	drawOverlay(guim);
	drawHealthbar();
	drawDebuginfo();
	drawAmmunition();
	drawChat();
	widgetDraw(rootHud,guim,0,0,screenWidth,screenHeight);
	if(isInventoryOpen()){
		drawInventory(guim);
	}
	textMeshDraw(guim);
}

void renderUI(){
	drawActiveItem();
	drawActiveGlider();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	shaderBind(sTextMesh);
	shaderMatrix(sTextMesh,matOrthoProj);
	drawCrosshair();
	drawHud();
	if(!mouseHidden){updateMouse();}

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

void guiCancel(){
	if(!gameRunning){return;}
	if(isInventoryOpen()){
		hideInventory();
		return;
	}
	if(widgetFocused == chatText){
		handlerRootHud(NULL);
		return;
	}
	menuCloseGame();
}
