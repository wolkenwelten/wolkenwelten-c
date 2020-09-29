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
#include "../voxel/bigchungus.h"
#include "../../../common/src/tmp/cto.h"
#include "../../../common/src/misc/misc.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

textMesh *menuM;

bool showAttribution  = false;
int  attributionLines = 0;

int   savegameCount = 0;
char  savegameName[16][32];

char menuTextInputLabel[32];

int   serverlistCount = 0;
char  serverlistName[16][32];
char  serverlistIP[16][64];

widget *rootMenu   = NULL;
widget *menuText   = NULL;
widget *mainMenu   = NULL;
widget *saveMenu   = NULL;
widget *saveList   = NULL;
widget *serverMenu = NULL;
widget *serverList = NULL;

widget *newGame     = NULL;
widget *newGameName = NULL;
widget *newGameSeed = NULL;

widget *newServer     = NULL;
widget *newServerName = NULL;
widget *newServerIP   = NULL;

widget *menuErrorLabel = NULL;

void startMultiplayer(){
	gameRunning     = true;
	connectionTries = 0;
	hideMouseCursor();
	rootMenu->flags |= WIDGET_HIDDEN;
	widgetFocus(NULL);
}
void startSingleplayer(){
	singleplayer    = true;
	startMultiplayer();
}

void handlerLoadGame(widget *wid);
void handlerDeleteGame(widget *wid);
static void refreshSaveList(){
	widgetEmpty(saveList);
	for(int i=0;i<savegameCount;i++){
		widget *button = widgetNewCPL(WIDGET_BUTTONDEL,saveList,16,i*48,256,32,savegameName[i]);
		widgetBind(button,"click",handlerLoadGame);
		widgetBind(button,"altclick",handlerDeleteGame);
		button->vali = i;
	}

	saveList->h = savegameCount * 48;
	widgetLayVert(saveMenu,16);
}

static void checkSavegames(){
	savegameCount = 0;
	DIR *dp = opendir("save/");
	if(dp == NULL){return;}
	struct dirent *de = NULL;
	while((de = readdir(dp)) != NULL){
		if(de->d_name[0] == '.'){continue;}
		snprintf(savegameName[savegameCount],sizeof(savegameName[0]),"%.31s",de->d_name);
		savegameName[savegameCount][sizeof(savegameName[0])-1] = 0;
		if(++savegameCount >= 16){break;}
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

void handlerJoinServer(widget *wid);
void handlerDeleteServer(widget *wid);
static void refreshServerList(){
	widgetEmpty(serverList);
	for(int i=0;i<serverlistCount;i++){
		widget *button = widgetNewCPL(WIDGET_BUTTONDEL,serverList,16,i*48,256,32,serverlistName[i]);
		widgetBind(button,"click",handlerJoinServer);
		widgetBind(button,"altclick",handlerDeleteServer);
		button->vali = i;
	}

	serverList->h = serverlistCount * 48;
	widgetLayVert(serverMenu,16);
}

static void checkServers(){
	refreshServerList();
}

static void addServer(const char *name, const char *ip){
	if(serverlistCount >= 15){return;}
	snprintf(serverlistName[serverlistCount],sizeof(serverlistName[0]),"%.31s",name);
	snprintf(serverlistIP[serverlistCount++],sizeof(serverlistIP[0]),"%.63s",ip);
}
static void delServer(int d){
	if(d < 0){return;}
	if(d >= serverlistCount){return;}
	for(int i=d;i<MIN(6,serverlistCount);i++){
		memcpy(serverlistName[i],serverlistName[i+1],32);
		memcpy(serverlistIP[i],serverlistIP[i+1],64);
	}
	serverlistCount--;
}
static void joinServer(int i){
	if(i < 0){return;}
	if(i >= serverlistCount){return;}
	snprintf(serverName,sizeof(serverName),"%.63s",serverlistIP[i]);
	startMultiplayer();
}

void handlerSingleplayer(widget *wid){
	(void)wid;
	checkSavegames();
	widgetSlideW(mainMenu,0);
	widgetSlideW(saveMenu,288);
	widgetFocus(NULL);
}
void handlerLoadGame(widget *wid){
	loadSavegame(wid->vali);
}
void handlerDeleteGame(widget *wid){
	deleteSavegame(wid->vali);
	checkSavegames();
}
void handlerJoinServer(widget *wid){
	joinServer(wid->vali);
}
void handlerDeleteServer(widget *wid){
	delServer(wid->vali);
	refreshServerList();
}
void handlerAttribution(widget *wid){
	(void)wid;
	showAttribution = true;
	menuText->flags |= WIDGET_HIDDEN;

	widgetSlideW(mainMenu,0);
	widgetSlideW(saveMenu,0);
	widgetSlideW(serverMenu,0);
	widgetFocus(NULL);
}
void handlerQuit(widget *wid){
	(void)wid;
	quit = true;
}
void handlerBackToMenu(widget *wid){
	(void)wid;
	showAttribution = false;
	menuText->flags &= ~WIDGET_HIDDEN;

	widgetSlideW(mainMenu,288);
	widgetSlideW(saveMenu,0);
	widgetSlideW(serverMenu,0);
	widgetSlideH(newGame,0);
	widgetSlideH(newServer,0);
	widgetFocus(NULL);
}
void handlerRoot(widget *wid){
	(void)wid;
	if(showAttribution){
		showAttribution = false;
		menuText->flags &= ~WIDGET_HIDDEN;
		widgetSlideW(mainMenu,288);
		widgetFocus(NULL);
	}
}

void handlerPlayerNameBlur(widget *wid){
	snprintf(playerName,sizeof(playerName),"%s",wid->vals);
}

void handlerNewGame(widget *wid){
	(void)wid;
	widgetSlideH(newGame,156);
	widgetFocus(newGameName);
}
void handlerNewGameCancel(widget *wid){
	(void)wid;
	widgetSlideH(newGame,0);
	newGameName->vals[0] = 0;
	newGameSeed->vals[0] = 0;
	widgetFocus(NULL);
}

void handlerNewGameSubmit(widget *wid){
	(void)wid;
	if(newGameName->vals[0] == 0){return;}
	snprintf(optionSavegame,sizeof(optionSavegame),"%s",newGameName->vals);
	if(newGameSeed->vals[0] != 0){
		optionWorldSeed = atoi(newGameSeed->vals);
	}
	handlerNewGameCancel(wid);
	startSingleplayer();
}

void handlerNewGameNext(widget *wid){
	(void)wid;
	widgetFocus(newGameSeed);
}

void handlerMultiplayer(widget *wid){
	(void)wid;
	checkServers();
	widgetSlideW(mainMenu,0);
	widgetSlideW(serverMenu,288);
	widgetFocus(NULL);
}
void handlerNewServer(widget *wid){
	(void)wid;
	widgetSlideH(newServer,156);
	widgetFocus(newServerName);
}
void handlerNewServerCancel(widget *wid){
	(void)wid;
	widgetSlideH(newServer,0);
	newServerName->vals[0] = 0;
	newServerIP->vals[0] = 0;
	widgetFocus(NULL);
}

void handlerNewServerSubmit(widget *wid){
	(void)wid;
	if((newServerName->vals[0] == 0) || (newServerIP->vals[0] == 0)){return;}
	addServer(newServerName->vals,newServerIP->vals);
	refreshServerList();
	handlerNewServerCancel(wid);
}

void handlerNewServerNext(widget *wid){
	(void)wid;
	widgetFocus(newServerIP);
}

void initMainMenu(){
	mainMenu = widgetNewCP(WIDGET_PANEL,rootMenu,-1,0,288,-1);
	widgetNewCP  (WIDGET_SPACE ,mainMenu,16,0,256,0);
	widgetNewCPLH(WIDGET_BUTTON,mainMenu,16,0,256,32,"Singleplayer","click",handlerSingleplayer);
	widgetNewCPLH(WIDGET_BUTTON,mainMenu,16,0,256,32,"Multiplayer","click",handlerMultiplayer);
	widgetNewCP  (WIDGET_SPACE ,mainMenu,16,0,256,32);
	widgetNewCPLH(WIDGET_BUTTON,mainMenu,16,0,256,32,"Attribution","click",handlerAttribution);
	widgetNewCPLH(WIDGET_BUTTON,mainMenu,16,0,256,32,"Quit","click",handlerQuit);

	widgetLayVert(mainMenu,16);
}

void initSaveMenu(){
	saveMenu = widgetNewCP(WIDGET_PANEL,rootMenu,-1,0,0,-1);
	saveMenu->flags |= WIDGET_HIDDEN;

	saveList = widgetNewCP(WIDGET_SPACE,saveMenu,0,0,288,32);
	widgetNewCPLH(WIDGET_BUTTON,saveMenu,16,0,256,32,"New Game","click",handlerNewGame);
	widgetNewCPLH(WIDGET_BUTTON,saveMenu,16,0,256,32,"Back to Menu","click",handlerBackToMenu);

	widgetLayVert(saveMenu,16);

	newGame = widgetNewCP(WIDGET_PANEL,rootMenu,32,-1,288,0);
	newGame->flags |= WIDGET_HIDDEN;
	newGameName = widgetNewCPLH(WIDGET_TEXTINPUT,newGame,16,16,256,32,"World Name","submit",handlerNewGameNext);
	newGameSeed = widgetNewCPLH(WIDGET_TEXTINPUT,newGame,16,64,256,32,"World Seed","submit",handlerNewGameSubmit);
	widgetNewCPLH(WIDGET_BUTTON,newGame,16,112,120,32,"Cancel","click",handlerNewGameCancel);
	widgetNewCPLH(WIDGET_BUTTON,newGame,148,112,120,32,"Create","click",handlerNewGameSubmit);
}

void initServerMenu(){
	serverMenu = widgetNewCP(WIDGET_PANEL,rootMenu,-1,0,0,-1);
	serverMenu->flags |= WIDGET_HIDDEN;

	serverList = widgetNewCP(WIDGET_SPACE,serverMenu,0,0,288,32);
	widgetNewCPLH(WIDGET_BUTTON,serverMenu,16,0,256,32,"New Server","click",handlerNewServer);
	widgetNewCPLH(WIDGET_BUTTON,serverMenu,16,0,256,32,"Back to Menu","click",handlerBackToMenu);
	widgetLayVert(serverMenu,16);

	newServer = widgetNewCP(WIDGET_PANEL,rootMenu,32,-1,288,0);
	newServer->flags |= WIDGET_HIDDEN;
	newServerName = widgetNewCPLH(WIDGET_TEXTINPUT,newServer,16,16,256,32,"Server Name","submit",handlerNewServerNext);
	newServerIP = widgetNewCPLH(WIDGET_TEXTINPUT,newServer,16,64,256,32,"IP / Domain","submit",handlerNewServerSubmit);
	widgetNewCPLH(WIDGET_BUTTON,newServer,16,112,120,32,"Cancel","click",handlerNewServerCancel);
	widgetNewCPLH(WIDGET_BUTTON,newServer,148,112,120,32,"Create","click",handlerNewServerSubmit);
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

	menuText = widgetNewCP(WIDGET_SPACE,rootMenu,32,32,256,-65);

	wid = widgetNewCPL(WIDGET_LABEL,menuText,0,0,256,32,"Wolkenwelten");
	wid->flags |= WIDGET_BIG;
	widgetNewCPL(WIDGET_LABEL,menuText,0,32,256,32,(char *)VERSION);
	wid = widgetNewCPL(WIDGET_LABEL,menuText,0,64,256,32,"Your Name: ");
	wid = widgetNewCPL(WIDGET_TEXTINPUT,menuText,0,88,256,32,"Playername");
	strncpy(wid->vals,playerName,256);
	widgetBind(wid,"blur",handlerPlayerNameBlur);

	menuErrorLabel = widgetNewCPL(WIDGET_LABEL,menuText,1,-97,256,16,"");
	widgetNewCPL(WIDGET_LABEL,menuText,1,-33,256,16,menuTextInputLabel);

	initMainMenu();
	initSaveMenu();
	initServerMenu();
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

void renderMenu(){
	shaderBind(sTextMesh);
	shaderMatrix(sTextMesh,matOrthoProj);
	if(mouseHidden){
		showMouseCursor();
	}
	updateMouse();

	textMeshEmpty(menuM);
	widgetDraw(rootMenu,menuM,0,0,screenWidth,screenHeight);
	drawMenuAttributions();
	textMeshDraw(menuM);

	drawCursor();
}

void menuChangeFocus(int xoff,int yoff){
	(void)xoff;
	if(gameRunning){return;}
	if(textInputActive()){return;}

	if(widgetFocused != NULL){
		if(yoff < 0){
			widgetFocus(widgetNextSel(widgetFocused));
		}else if(yoff > 0){
			widgetFocus(widgetPrevSel(widgetFocused));
		}
	}
	if(widgetFocused == NULL){
		if(yoff < 0){
			widgetFocus(widgetNextSel(rootMenu));
		}else if(yoff > 0){
			widgetFocus(widgetPrevSel(rootMenu));
		}
	}
}

void menuKeyClick(int btn){
	(void)btn;
	if(gameRunning){return;}
	if(showAttribution){widgetEmit(rootMenu,"click");return;}
	if(widgetFocused == NULL){return;}
	if(btn == 1){
		widgetEmit(widgetFocused,"altclick");
	}else{
		widgetEmit(widgetFocused,"click");
	}
}

void menuSetError(char *error){
	menuErrorLabel->vals = error;
}

void menuCancel(){
	if(gameRunning){return;}

	if(showAttribution){
		widgetEmit(rootMenu,"click");
		return;
	}
	if((saveMenu->gw > 0) || (serverMenu->gw > 0)){
		if(newGame->gh > 0){
			handlerNewGameCancel(NULL);
		}else if(newServer->gh > 0){
			handlerNewServerCancel(NULL);
		}else{
			handlerBackToMenu(NULL);
		}
		return;
	}
	quit = true;
}

void menuCloseGame(){
	gameRunning=false;
	rootMenu->flags &= ~WIDGET_HIDDEN;
	clientGoodbye();
	clientFree();
	bigchungusFree(&world);
}
