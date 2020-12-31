#include "gui.h"

#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"

#include "../main.h"
#include "../misc/lisp.h"
#include "../misc/options.h"
#include "../game/animal.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../game/time.h"
#include "../sdl/sdl.h"
#include "../gfx/gl.h"
#include "../gfx/mesh.h"
#include "../gfx/gfx.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"
#include "../gfx/mat.h"
#include "../gfx/texture.h"
#include "../gfx/textMesh.h"
#include "../gfx/sky.h"
#include "../gui/menu.h"
#include "../misc/options.h"
#include "../menu/inventory.h"
#include "../menu/mainmenu.h"
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

widget *widgetGameScreen;
widget *chatPanel;
widget *chatText;
widget *lispPanel;
widget *lispLog;
widget *lispInput;
int lispHistoryActive = -1;

bool mouseHidden = false;
uint mousex,mousey;
uint mouseClicked[3] = {0,0,0};
uint animalOverlaysDrawn = 0;
bool lispPanelVisible = false;

float matOrthoProj[16];

void handlerRootHud(widget *wid){
	(void)wid;
	chatText->vals[0]  = 0;
	widgetFocus(widgetGameScreen);
	widgetSlideH(chatPanel, 0);
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
	matOrtho(matOrthoProj,0.f,screenWidth,screenHeight,0.f,-1.f,16.f);

	const int sx = 10*getTilesize();
	chatPanel->w = screenWidth - sx;
	chatText->w  = screenWidth - sx - 64;
	lispPanel->w = screenWidth - 128;
	if(lispPanelVisible){
		widgetSlideH(lispPanel, screenHeight-128);
	}

	initInventory();
}

void openChat(){
	if(gameControlsInactive()){return;}
	widgetSlideH(chatPanel, 64);
	widgetFocus(chatText);
}

void openLispPanel(){
	widgetSlideH(lispPanel, screenHeight-128);
	widgetFocus(lispInput);
	lispPanelVisible = true;
}

void closeLispPanel(){
	widgetSlideH(lispPanel, 0);
	if(gameRunning){
		widgetFocus(widgetGameScreen);
	}
	lispPanelVisible = false;
}

void lispPanelShowReply(lVal *sym, const char *reply){
	int len = strnlen(reply,256) + 1;
	if(lispLog == NULL){return;}
	for(int i=0;i<256;i++){
		const char *line = lispLog->valss[i];
		if(line == NULL)                  {continue;}
		if(*line != ' ')                  {continue;}
		for(;*line == ' ';line++){}
		if(strncmp(sym->vSymbol.c,line,6)){
			printf("'%s' != '%s'\n",line,sym->vSymbol.c);
			continue;
		}
		free(lispLog->valss[i]);
		lispLog->valss[i] = malloc(len+3);
		for(int ii=0;ii<3;ii++){lispLog->valss[i][ii] = ' ';}
		memcpy(&lispLog->valss[i][3],reply,len);
		lispLog->valss[i][len+3] = 0;
		break;
	}
	fprintf(stderr,"Couldn't match SExpr Reply %s - %s\n",sym->vSymbol.c,reply);
}

void toggleLispPanel(){
	if(lispPanelVisible){
		closeLispPanel();
	}else{
		openLispPanel();
	}
}

void handlerLispSubmit(widget *wid){
	static char buf[512];
	if(lispInput->vals[0] == 0){return;}
	snprintf(buf,sizeof(buf)-1,"> %s",wid->vals);
	widgetAddEntry(lispLog, buf);
	const char *result = lispEval(wid->vals);
	snprintf(buf,sizeof(buf)-1,"   %s",result);
	widgetAddEntry(lispLog, buf);
	wid->vals[0] = 0;
	lispHistoryActive = -1;
	textInputFocus(wid);
}

void handlerLispSelectPrev(widget *wid){
	const char *msg = lispLog->valss[lispHistoryActive+1];
	if(msg == NULL){return;}
	if(*msg == 0)  {return;}
	++lispHistoryActive;
	if(*msg == ' '){
		handlerLispSelectPrev(wid);
		return;
	}
	int firstSpace;
	for(firstSpace=0;firstSpace<255;firstSpace++){
		if(msg[firstSpace] == ' '){break;}
	}
	firstSpace++;
	memcpy(wid->vals,&msg[firstSpace],255-firstSpace);
	wid->vals[255]=0;
	textInputFocus(wid);
}

void handlerLispSelectNext(widget *wid){
	if(lispHistoryActive < 0){return;}
	const char *msg = lispLog->valss[lispHistoryActive-1];
	if(lispHistoryActive <= 0){
		memset(wid->vals,0,256);
		textInputFocus(wid);
		return;
	}
	if(msg == NULL){return;}
	if(*msg == 0)  {return;}
	--lispHistoryActive;
	if(*msg == ' '){
		handlerLispSelectNext(wid);
		return;
	}
	int firstSpace;
	for(firstSpace=0;firstSpace<255;firstSpace++){
		if(wid->vals[firstSpace] == ' '){break;}
	}
	memcpy(wid->vals,&msg[firstSpace],255-firstSpace);
	wid->vals[255]=0;
	textInputFocus(wid);
}

static void handlerChatChange(widget *wid){
	if(wid == NULL){return;}
	if(wid->vals == NULL){return;}
	if(wid->vals[0] == '.'){
		wid->flags |= WIDGET_LISP_SYNTAX_HIGHLIGHT;
	}else{
		wid->flags &= ~WIDGET_LISP_SYNTAX_HIGHLIGHT;
	}
}

void handlerChatSubmit(widget *wid){
	if(chatText->vals[0] != 0){
		msgSendChatMessage(chatText->vals);
		chatResetHistorySel();
	}
	handlerRootHud(wid);
	wid->flags &= ~WIDGET_LISP_SYNTAX_HIGHLIGHT;
}

void handlerChatSelectPrev(widget *wid){
	const char *msg = chatGetPrevHistory();
	if(*msg == 0){return;}
	memcpy(wid->vals,msg,256);
	textInputFocus(wid);
	handlerChatChange(wid);
}

void handlerChatSelectNext(widget *wid){
	const char *msg = chatGetNextHistory();
	if(*msg == 0){return;}
	memcpy(wid->vals,msg,256);
	textInputFocus(wid);
	handlerChatChange(wid);
}

void handlerChatBlur(widget *wid){
	chatResetHistorySel();
	wid->flags &= ~WIDGET_LISP_SYNTAX_HIGHLIGHT;
}

static void handlerGameFocus(widget *wid){
	(void)wid;
	showInventoryPanel();
}

void initUI(){
	cursorMesh           = textMeshNew(8);
	cursorMesh->tex      = tCursor;

	crosshairMesh        = textMeshNew(16);
	crosshairMesh->tex   = tCrosshair;
	crosshairMesh->usage = GL_STREAM_DRAW;

	guim                 = textMeshNew(1<<16);
	guim->tex            = tGui;

	widgetGameScreen = widgetNewCP(wGameScreen,rootMenu,0,0,-1,-1);
	widgetBind(widgetGameScreen,"focus",handlerGameFocus);
	chatPanel = widgetNewCP(wPanel,rootMenu,0,-1,512,0);
	chatPanel->flags |= WIDGET_HIDDEN;

	lispPanel = widgetNewCP(wPanel,rootMenu,64,0,screenWidth-128,0);
	lispPanel->flags |= WIDGET_HIDDEN;
	lispInput = widgetNewCP(wTextInput,lispPanel,0,-1,-1,32);
	lispInput->flags |= WIDGET_LISP_SYNTAX_HIGHLIGHT;
	lispLog = widgetNewCP(wTextLog,lispPanel,0,0,-1,-32);
	lispLog->flags |= WIDGET_LISP_SYNTAX_HIGHLIGHT;
	widgetBind(lispInput,"submit",handlerLispSubmit);
	widgetBind(lispInput,"selectPrev",handlerLispSelectPrev);
	widgetBind(lispInput,"selectNext",handlerLispSelectNext);

	chatText  = widgetNewCPLH(wTextInput,chatPanel,16,16,440,32,"Message","submit",handlerChatSubmit);
	widgetBind(chatText,"change",handlerChatChange);
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
	vec p = c->screenPos;
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

static void drawHookIndicator(){
	const float hookdist = characterCanHookHit(player);
	if(hookdist < 0.f){return;}
	textMeshItemSprite(guim,screenWidth/2+8,screenHeight/2+8,32,I_Hook);
	textMeshPrintfPS(guim,screenWidth/2 + 40, screenHeight/2 + 16,2,"%.1f",hookdist);
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
	guim->size = 2;
	guim->sx   = screenWidth;
	guim->sy   = 4;
	textMeshPrintfRA(guim,"%s",VERSION);
	drawHookIndicator();

	guim->sx   = screenWidth-96;
	guim->sy   = 24;
	textMeshPrintf(guim,"FPS %s%02.0f\n",colorSignalHigh(20,40,59,curFPS),curFPS);
	guim->fgc  = colorPalette[15];

	guim->sy  += 16;
	guim->sx   = screenWidth;
	guim->size = 2;
	for(uint i=0;i<32;i++){
		const char *cname = characterGetPlayerName(i);
		if(i == (uint)playerID){continue;}
		if(cname == NULL){continue;}
		guim->sx = screenWidth;
		guim->sy = 32+(i*42);
		textMeshPrintfRA(guim,"%s",cname);
		drawSingleHealthbar(characterGetPlayerHP(i),20,screenWidth-96,guim->sy+22,14,false);
		drawPlayerOverlay(i);
	}

	vboTrisCount = 0;
	if(optionDebugInfo){
		guim->font = 1;
		guim->sx   = 4;
		guim->sy   = getTilesize();
		guim->size = 2;
		const float compRatio = (float)recvUncompressedBytesCurrentSession / (float)recvBytesCurrentSession;
		textMeshPrintf(guim,"Player     X: %05.2f VX: %02.4f GVX: %02.4f\n",player->pos.x,player->vel.x,player->gvel.x);
		textMeshPrintf(guim,"Player     Y: %05.2f VY: %02.4f GVY: %02.4f\n",player->pos.y,player->vel.y,player->gvel.y);
		textMeshPrintf(guim,"Player     Z: %05.2f VZ: %02.4f GVZ: %02.4f\n",player->pos.z,player->vel.z,player->gvel.z);
		textMeshPrintf(guim,"Player   YPR: %04.2f %04.2f %04.2f\n",player->rot.yaw,player->rot.pitch,player->rot.roll);
		textMeshPrintf(guim,"PlayerChungi: %u %u %u\n",((uint)player->pos.x)>>8,((uint)player->pos.y)>>8,((uint)player->pos.z)>>8);
		textMeshPrintf(guim,"Active Tris.: %s%s\n",colorSignalLow(1<<21,1<<19,1<<18,tris),getHumanReadableSize(tris));
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Particles   : %s%s\n",colorSignalLow(1<<16,1<<15,1<<14,particleCount),getHumanReadableSize(particleCount));
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Sparticles  : %s%s\n",colorSignalLow(1<<15,1<<14,1<<13,sparticleCount),getHumanReadableSize(sparticleCount));
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Chunks gener: %2i\n",chunkGetGeneratedThisFrame());
		textMeshPrintf(guim,"ActiveChunks: %s\n",getHumanReadableSize(chunkGetActive()));
		textMeshPrintf(guim,"FreeChunks  : %2i\n",chunkGetFree());
		textMeshPrintf(guim,"ActiveChungi: %2i\n",chungusGetActiveCount());
		textMeshPrintf(guim,"Bytes Sent  : %sB\n",getHumanReadableSize(sentBytesCurrentSession));
		textMeshPrintf(guim,"Bytes Recvd : %sB\n",getHumanReadableSize(recvBytesCurrentSession));
		textMeshPrintf(guim,"Uncompressed: %sB\n",getHumanReadableSize(recvUncompressedBytesCurrentSession));
		textMeshPrintf(guim,"Comp. Ratio : %s%2.2fX\n",colorSignalHigh(4,8,15,compRatio),compRatio);
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Canvas Size : %ix%i\n",screenWidth,screenHeight);
		guim->size =  2;
		textMeshPrintf(guim,"Ping  : %s%u\n",colorSignalLow(400,200,50,lastLatency),lastLatency);
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"WorstF: %s%u\n",colorSignalLow(60,20,18,worstFrame),worstFrame);
		guim->fgc  = colorPalette[15];
		textMeshPrintf(guim,"Player: %u\n",playerID);
		textMeshPrintf(guim,"Time  : %s\n",gtimeGetTimeOfDayHRS(gtimeGetTimeOfDay()));
		textMeshPrintf(guim,"Bright: %f\n",skyBrightness);

		animalOverlaysDrawn = 0;
		for(uint i=0;i<animalCount;i++){
			drawAnimalDebugOverlay(&animalList[i],i);
		}
	}
	guim->font = 0;
}

void drawActiveItem(){
	float matViewAI[16];
	item *activeItem = &player->inventory[player->activeItem];
	if(activeItem == NULL){return;}
	if(itemIsEmpty(activeItem)){return;}

	mesh *aiMesh = getMeshDispatch(activeItem);
	if(aiMesh == NULL){return;}

	const float ix =  (1.5f * (1.f - player->aimFade));
	float iy = -1.2f + (0.3f * player->aimFade);
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

	const float shake = MINMAX(0.f,0.2f,player->shake);
	float deg  = ((float)++ticks*0.4f);
	float yoff = cos(deg * 2.1f) * shake;
	float xoff = sin(deg * 1.3f) * shake;

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
	guim->wrap = 0;

	drawOverlay(guim);
	drawHealthbar();
	drawDebuginfo();
	drawAmmunition();
	drawChat();
	widgetDraw(rootMenu,guim,0,0,screenWidth,screenHeight);
	if(isInventoryOpen()){
		drawInventory(guim);
	}
	textMeshDraw(guim);
}

void renderUI(){
	if(!optionThirdPerson){
		drawActiveItem();
		drawActiveGlider();
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	shaderBind(sTextMesh);
	shaderMatrix(sTextMesh,matOrthoProj);
	if((widgetFocused != NULL) && (widgetFocused->type == wGameScreen)){
		hideMouseCursor();
	}else{
		showMouseCursor();
	}
	drawHud();
	drawCrosshair();
	updateMouse();

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
	if(lispPanelVisible){
		closeLispPanel();
		return;
	}
	if(widgetFocused == widgetGameScreen){
		openMainMenu();
		widgetFocus(NULL);
		return;
	}
	closeAllMenus();
	widgetFocus(widgetGameScreen);
}
