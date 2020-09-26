#include "menu.h"

#include "../main.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../gfx/gl.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/textMesh.h"
#include "../network/client.h"
#include "../misc/options.h"
#include "../tmp/assets.h"
#include "../../../common/src/tmp/cto.h"
#include "../../../common/src/misc/misc.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

textMesh *menuM;
char *menuError = "";

bool showAttribution  = false;
bool showSavegames    = false;

int  attributionLines = 0;
static int gamepadSelection = -1;

int   savegameCount = 0;
char  savegameName[8][32];

widget *rootMenu = NULL;
widget *menuText = NULL;
widget *mainMenu = NULL;
widget *saveMenu = NULL;
widget *saveList = NULL;

void startSingleplayer();
void handlerLoadGame(widget *wid);
void handlerDeleteGame(widget *wid);
static void refreshSaveList(){
	widgetEmpty(saveList);
	for(int i=0;i<savegameCount;i++){
		widget *button;

		button = widgetNewCPLH(WIDGET_BUTTON,saveList,64,i*48,208,32,savegameName[i],"click",handlerLoadGame);
		button->vali = i;
		button = widgetNewCPLH(WIDGET_BUTTON,saveList,16,i*48,32,32,"X","click",handlerDeleteGame);
		button->vali = i;
	}

	saveList->h = savegameCount*48 + 48;
	widgetLayVert(saveMenu,16);
}

static void checkSavegames(){
	showSavegames = true;
	if(gamepadSelection != -1){
		gamepadSelection = 0;
	}
	savegameCount = 0;
	DIR *dp = opendir("save/");
	if(dp == NULL){return;}
	struct dirent *de = NULL;
	while((de = readdir(dp)) != NULL){
		if(de->d_name[0] == '.'){continue;}
		snprintf(savegameName[savegameCount++],sizeof(savegameName[0]),"%.31s",de->d_name);
		savegameName[savegameCount-1][sizeof(savegameName[0])-1] = 0;
	}
	closedir(dp);
	refreshSaveList();
}

static void loadSavegame(int i){
	if(i < 0){return;}
	if(i >= savegameCount){return;}
	strncpy(optionSavegame,savegameName[i],32);
	optionSavegame[31] = 0;
	startSingleplayer();
}

static void deleteSavegame(int i){
	static char buf[64];
	if(i < 0)             {return;}
	if(i > savegameCount) {return;}
	snprintf(buf,64,"save/%s",savegameName[i]);
	buf[63]=0;
	rmDirR(buf);
}

void handlerSingleplayer(widget *wid){
	(void)wid;
	checkSavegames();
	mainMenu->flags &= ~WIDGET_HIDDEN;
	mainMenu->flags |= WIDGET_ANIMATEW;
	mainMenu->gw     =   0;
	saveMenu->flags &= ~WIDGET_HIDDEN;
	saveMenu->flags |= WIDGET_ANIMATEW;
	saveMenu->gw     = 288;
}
void handlerMultiplayer(widget *wid){
	(void)wid;
	textInput(8,screenHeight-24,256,16,2);
}
void handlerChangeName(widget *wid){
	(void)wid;
	textInput(8,screenHeight-24,256,16,3);
}
void handlerLoadGame(widget *wid){
	loadSavegame(wid->vali);
}
void handlerDeleteGame(widget *wid){
	deleteSavegame(wid->vali);
	checkSavegames();
}
void handlerNewGame(widget *wid){
	(void)wid;
	textInput(8,screenHeight-24,256,16,4);
}
void handlerAttribution(widget *wid){
	(void)wid;
	showAttribution = true;
	menuText->flags |= WIDGET_HIDDEN;

	mainMenu->flags &= ~WIDGET_HIDDEN;
	mainMenu->flags |= WIDGET_ANIMATEW;
	mainMenu->gw     =   0;
	saveMenu->flags &= ~WIDGET_HIDDEN;
	saveMenu->flags |= WIDGET_ANIMATEW;
	saveMenu->gw     =   0;

}
void handlerQuit(widget *wid){
	(void)wid;
	quit = true;
}
void handlerBackToMenu(widget *wid){
	(void)wid;
	showAttribution = false;
	menuText->flags &= ~WIDGET_HIDDEN;

	mainMenu->flags &= ~WIDGET_HIDDEN;
	mainMenu->flags |= WIDGET_ANIMATEW;
	mainMenu->gw     = 288;
	saveMenu->flags &= ~WIDGET_HIDDEN;
	saveMenu->flags |= WIDGET_ANIMATEW;
	saveMenu->gw     =   0;
}
void handlerRoot(widget *wid){
	(void)wid;
	if(showAttribution){
		showAttribution = false;
		menuText->flags &= ~WIDGET_HIDDEN;
		saveMenu->flags |=  WIDGET_HIDDEN;
		mainMenu->flags &= ~WIDGET_HIDDEN;
		mainMenu->flags |= WIDGET_ANIMATEW;
		mainMenu->gw     = 288;
	}
}

void initMainMenu(){
	mainMenu = widgetNewCP(WIDGET_PANEL,rootMenu,-1,0,288,-1);
	widgetNewCPLH(WIDGET_BUTTON,mainMenu,16,0,256,32,"Singleplayer","click",handlerSingleplayer);
	widgetNewCPLH(WIDGET_BUTTON,mainMenu,16,0,256,32,"Multiplayer","click",handlerMultiplayer);
	widgetNewCP  (WIDGET_SPACE ,mainMenu,16,0,256,32);
	widgetNewCPLH(WIDGET_BUTTON,mainMenu,16,0,256,32,"Change Name","click",handlerChangeName);
	widgetNewCPLH(WIDGET_BUTTON,mainMenu,16,0,256,32,"Attribution","click",handlerAttribution);
	widgetNewCP  (WIDGET_SPACE ,mainMenu,16,0,256,32);
	widgetNewCPLH(WIDGET_BUTTON,mainMenu,16,0,256,32,"Quit","click",handlerQuit);

	widgetLayVert(mainMenu,16);
}

void initSaveMenu(){
	saveMenu = widgetNewCP(WIDGET_PANEL,rootMenu,-1,0,0,-1);
	saveMenu->flags |= WIDGET_HIDDEN;

	saveList = widgetNewCP(WIDGET_SPACE,saveMenu,-1,0,288,32);
	widgetNewCP  (WIDGET_SPACE, saveMenu,16,0,256,32);
	widgetNewCPLH(WIDGET_BUTTON,saveMenu,16,0,256,32,"New Game","click",handlerNewGame);
	widgetNewCPLH(WIDGET_BUTTON,saveMenu,16,0,256,32,"Back to Menu","click",handlerBackToMenu);

	widgetLayVert(saveMenu,16);
}

void initMenu(){
	widget *wid;
	menuM = textMeshNew();
	menuM->tex = tGui;

	attributionLines = 0;
	unsigned char *s = txt_attribution_txt_data;
	while(*s++ != 0){
		if(*s == '\n'){attributionLines++;}
	}

	rootMenu = widgetNewCP(WIDGET_BACKGROUND,NULL,0,0,-1,-1);
	widgetBind(rootMenu,"click",handlerRoot);

	menuText = widgetNewCP(WIDGET_SPACE,rootMenu,32,32,256,256);

	wid = widgetNewCPL(WIDGET_LABEL,menuText,0,0,256,32,"Wolkenwelten");
	wid->vali = 4;
	wid = widgetNewCPL(WIDGET_LABEL,menuText,0,32,256,32,(char *)VERSION);
	wid->vali = 2;
	wid = widgetNewCPL(WIDGET_LABEL,menuText,0,64,256,32,"Your Name:");
	wid->vali = 2;
	wid = widgetNewCPL(WIDGET_LABEL,menuText,176,64,256,32,playerName);
	wid->vali = 2;

	initMainMenu();
	initSaveMenu();
}

void startSingleplayer(){
	singleplayer    = true;
	gameRunning     = true;
	textInputLock   = 0;
	textInputActive = false;
	connectionTries = 0;
	hideMouseCursor();
}

void updateMenu(){
	if(!textInputActive && (textInputLock == 2)){
		char *buf = textInputGetBuffer();
		strncpy(serverName,buf,sizeof(serverName)-1);
		textInputLock   = 0;
		connectionTries = 0;
		gameRunning     = true;
		hideMouseCursor();
	}

	if(!textInputActive && (textInputLock == 3)){
		snprintf(playerName,sizeof(playerName),"%s",textInputGetBuffer());
		textInputLock = 0;
	}

	if(!textInputActive && (textInputLock == 4)){
		snprintf(optionSavegame,sizeof(optionSavegame),"%s",textInputGetBuffer());
		textInputLock = 0;
		startSingleplayer();
	}
}

void drawMenuAttributions(){
	static int scroll    = 0;
	static int scrollDir = 1;
	const int textHeight = attributionLines * 16;

	if(!showAttribution){return;}

	if(scroll > textHeight-screenHeight){
		scrollDir=-1;
	}
	if(scroll < 0){
		scrollDir=1;
	}
	scroll+=scrollDir;

	textMeshPrintfPS(menuM,16,16-scroll,2,"Attribution:\n%s",txt_attribution_txt_data);
}

void drawMenuTextInputs(){
	if(textInputActive && (textInputLock == 2)){
		textMeshAddStrPS(menuM,8,screenHeight-42,2,"Servername:");
	}
	if(textInputActive && (textInputLock == 3)){
		textMeshAddStrPS(menuM,8,screenHeight-42,2,"Your Name:");
	}
	if((menuError != NULL) && (*menuError != 0)){
		textMeshAddStrPS(menuM,8,screenHeight-58,2,menuError);
	}
}

void renderMenu(){
	shaderBind(sTextMesh);
	shaderMatrix(sTextMesh,matOrthoProj);
	if(mouseHidden){
		showMouseCursor();
	}
	updateMouse();
	updateMenu();

	textMeshEmpty(menuM);
	drawMenuTextInputs();
	widgetDraw(rootMenu,menuM,0,0,screenWidth,screenHeight);
	drawMenuAttributions();
	textMeshDraw(menuM);

	textInputDraw();
	drawCursor();
}
