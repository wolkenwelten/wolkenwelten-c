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

int  savegameCount = 0;
char savegameName[16][32];

char menuTextInputLabel[32];

int  serverlistCount = 0;
char serverlistName[16][32];
char serverlistIP[16][64];

widget *rootMenu;

widget *menuText;
widget *mainMenu;

widget *saveMenu;
widget *saveList;
widget *newGame;
widget *newGameName;
widget *newGameSeed;

widget *serverMenu;
widget *serverList;
widget *newServer;
widget *newServerName;
widget *newServerIP;

widget *optionsMenu;
widget *optionsName;
widget *optionsVolume;
widget *menuErrorLabel;

void startMultiplayer(){
	gameRunning     = true;
	connectionTries = 0;

	mainMenu->flags |= WIDGET_HIDDEN;
	mainMenu->w = mainMenu->gw = 0;
	saveMenu->flags |= WIDGET_HIDDEN;
	saveMenu->w = saveMenu->gw = 0;
	serverMenu->flags &= ~WIDGET_HIDDEN;
	serverMenu->w = serverMenu->gw = 288;

	rootMenu->flags |= WIDGET_HIDDEN;
	widgetFocus(NULL);
	hideMouseCursor();
}
void startSingleplayer(){
	singleplayer    = true;
	startMultiplayer();

	serverMenu->flags |= WIDGET_HIDDEN;
	serverMenu->w = serverMenu->gw = 0;
	saveMenu->flags &= ~WIDGET_HIDDEN;
	saveMenu->w = saveMenu->gw = 288;

}

static void handlerLoadGame(widget *wid);
static void handlerDeleteGame(widget *wid);
static void refreshSaveList(){
	widgetEmpty(saveList);
	for(int i=0;i<savegameCount;i++){
		widget *button = widgetNewCPL(wButtonDel,saveList,16,i*48,256,32,savegameName[i]);
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

static void handlerJoinServer(widget *wid);
static void handlerDeleteServer(widget *wid);
static void refreshServerList(){
	widgetEmpty(serverList);
	for(int i=0;i<serverlistCount;i++){
		widget *button = widgetNewCPL(wButtonDel,serverList,16,i*48,256,32,serverlistName[i]);
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

static void handlerSingleplayer(widget *wid){
	(void)wid;
	checkSavegames();
	widgetSlideW(mainMenu,0);
	widgetSlideW(saveMenu,288);
	widgetFocus(NULL);
}
static void handlerLoadGame(widget *wid){
	loadSavegame(wid->vali);
}
static void handlerDeleteGame(widget *wid){
	deleteSavegame(wid->vali);
	checkSavegames();
}
static void handlerJoinServer(widget *wid){
	joinServer(wid->vali);
}
static void handlerDeleteServer(widget *wid){
	delServer(wid->vali);
	refreshServerList();
	saveOptions();
}
static void handlerAttribution(widget *wid){
	(void)wid;
	showAttribution = true;
	menuText->flags |= WIDGET_HIDDEN;

	widgetSlideW(mainMenu,0);
	widgetSlideW(saveMenu,0);
	widgetSlideW(serverMenu,0);
	widgetFocus(NULL);
}
static void handlerQuit(widget *wid){
	(void)wid;
	quit = true;
}
static void handlerBackToMenu(widget *wid){
	(void)wid;
	showAttribution = false;
	menuText->flags &= ~WIDGET_HIDDEN;

	widgetSlideW(mainMenu,288);
	widgetSlideW(saveMenu,0);
	widgetSlideW(serverMenu,0);
	widgetSlideW(optionsMenu,0);
	widgetSlideH(newGame,0);
	widgetSlideH(newServer,0);
	widgetFocus(NULL);
}
static void handlerRoot(widget *wid){
	(void)wid;
	if(showAttribution){
		showAttribution = false;
		menuText->flags &= ~WIDGET_HIDDEN;
		widgetSlideW(mainMenu,288);
		widgetFocus(NULL);
	}
}

static void handlerNewGame(widget *wid){
	(void)wid;
	widgetSlideH(newGame,156);
	widgetFocus(newGameName);
}
static void handlerNewGameCancel(widget *wid){
	(void)wid;
	widgetSlideH(newGame,0);
	newGameName->vals[0] = 0;
	newGameSeed->vals[0] = 0;
	widgetFocus(NULL);
}

static void handlerNewGameSubmit(widget *wid){
	(void)wid;
	if(newGameName->vals[0] == 0){return;}
	snprintf(optionSavegame,sizeof(optionSavegame),"%s",newGameName->vals);
	if(newGameSeed->vals[0] != 0){
		optionWorldSeed = atoi(newGameSeed->vals);
	}
	handlerNewGameCancel(wid);
	startSingleplayer();
}

static void handlerNewGameNext(widget *wid){
	(void)wid;
	widgetFocus(newGameSeed);
}

static void handlerMultiplayer(widget *wid){
	(void)wid;
	checkServers();
	widgetSlideW(mainMenu,0);
	widgetSlideW(serverMenu,288);
	widgetFocus(NULL);
}
static void handlerNewServer(widget *wid){
	(void)wid;
	widgetSlideH(newServer,156);
	widgetFocus(newServerName);
}
static void handlerNewServerCancel(widget *wid){
	(void)wid;
	widgetSlideH(newServer,0);
	newServerName->vals[0] = 0;
	newServerIP->vals[0] = 0;
	widgetFocus(NULL);
}
static void handlerNewServerSubmit(widget *wid){
	(void)wid;
	if((newServerName->vals[0] == 0) || (newServerIP->vals[0] == 0)){return;}
	addServer(newServerName->vals,newServerIP->vals);
	refreshServerList();
	saveOptions();
	handlerNewServerCancel(wid);
}
static void handlerNewServerNext(widget *wid){
	(void)wid;
	widgetFocus(newServerIP);
}
static void handlerOptions(widget *wid){
	(void)wid;
	checkServers();
	widgetSlideW(mainMenu,0);
	widgetSlideW(optionsMenu,288);
	widgetFocus(NULL);
}

static void initMainMenu(){
	mainMenu = widgetNewCP(wPanel,rootMenu,-1,0,288,-1);
	widgetNewCP  (wSpace ,mainMenu,16,0,256,0);
	widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Singleplayer","click",handlerSingleplayer);
	widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Multiplayer","click",handlerMultiplayer);
	widgetNewCP  (wHR ,mainMenu,16,0,256,32);
	widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Options","click",handlerOptions);
	widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Attribution","click",handlerAttribution);
	widgetNewCP  (wHR ,mainMenu,16,0,256,32);
	widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Quit","click",handlerQuit);

	widgetLayVert(mainMenu,16);
}

static void initSaveMenu(){
	saveMenu = widgetNewCP(wPanel,rootMenu,-1,0,0,-1);
	saveMenu->flags |= WIDGET_HIDDEN;

	saveList = widgetNewCP(wSpace,saveMenu,0,0,288,32);
	widgetNewCP(wHR,saveMenu,16,0,256,32);
	widgetNewCPLH(wButton,saveMenu,16,0,256,32,"New Game","click",handlerNewGame);
	widgetNewCPLH(wButton,saveMenu,16,0,256,32,"Back to Menu","click",handlerBackToMenu);

	widgetLayVert(saveMenu,16);

	newGame = widgetNewCP(wPanel,rootMenu,32,-1,288,0);
	newGame->flags |= WIDGET_HIDDEN;
	newGameName = widgetNewCPLH(wTextInput,newGame,16,16,256,32,"World Name","submit",handlerNewGameNext);
	newGameSeed = widgetNewCPLH(wTextInput,newGame,16,64,256,32,"World Seed","submit",handlerNewGameSubmit);
	widgetNewCPLH(wButton,newGame,16,112,120,32,"Cancel","click",handlerNewGameCancel);
	widgetNewCPLH(wButton,newGame,148,112,120,32,"Create","click",handlerNewGameSubmit);
	checkSavegames();
}

static void initServerMenu(){
	serverMenu = widgetNewCP(wPanel,rootMenu,-1,0,0,-1);
	serverMenu->flags |= WIDGET_HIDDEN;

	serverList = widgetNewCP(wSpace,serverMenu,0,0,288,32);
	widgetNewCP(wHR,serverMenu,16,0,256,32);
	widgetNewCPLH(wButton,serverMenu,16,0,256,32,"New Server","click",handlerNewServer);
	widgetNewCPLH(wButton,serverMenu,16,0,256,32,"Back to Menu","click",handlerBackToMenu);
	widgetLayVert(serverMenu,16);

	newServer = widgetNewCP(wPanel,rootMenu,32,-1,288,0);
	newServer->flags |= WIDGET_HIDDEN;
	newServerName = widgetNewCPLH(wTextInput,newServer,16,16,256,32,"Server Name","submit",handlerNewServerNext);
	newServerIP = widgetNewCPLH(wTextInput,newServer,16,64,256,32,"IP / Domain","submit",handlerNewServerSubmit);
	widgetNewCPLH(wButton,newServer,16,112,120,32,"Cancel","click",handlerNewServerCancel);
	widgetNewCPLH(wButton,newServer,148,112,120,32,"Create","click",handlerNewServerSubmit);
	checkServers();
}

static void handlerOptionsSave(widget *wid){
	optionSoundVolume = optionsVolume->vali / 4096.f;
	snprintf(playerName,sizeof(playerName),"%s",optionsName->vals);
	saveOptions();
	handlerBackToMenu(wid);
}
static void handlerOptionsCancel(widget *wid){
	optionsVolume->vali = optionSoundVolume * 4096.f;
	snprintf(optionsName->vals,256,"%s",playerName);
	handlerBackToMenu(wid);
}

static void initOptionsMenu(){
	optionsMenu = widgetNewCP(wPanel,rootMenu,-1,0,0,-1);
	optionsMenu->flags |= WIDGET_HIDDEN;

	widgetNewCP  (wSpace ,optionsMenu,16,0,256,0);

	optionsName = widgetNewCPL(wTextInput,optionsMenu,16,0,256,32,"Playername");
	strncpy(optionsName->vals,playerName,256);
	optionsVolume = widgetNewCPL(wSlider,optionsMenu,16,0,256,32,"Volume");
	optionsVolume->vali = optionSoundVolume * 4096.f;
	widgetNewCP  (wHR ,optionsMenu,16,0,256,32);
	widgetNewCPLH(wButton,optionsMenu,16,0,256,32,"Save","click",handlerOptionsSave);
	widgetNewCPLH(wButton,optionsMenu,16,0,256,32,"Cancel","click",handlerOptionsCancel);
	widgetLayVert(optionsMenu,16);
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

	rootMenu = widgetNewCP(wBackground,NULL,0,0,-1,-1);
	widgetBind(rootMenu,"click",handlerRoot);

	menuText = widgetNewCP(wSpace,rootMenu,32,32,256,-65);

	wid = widgetNewCPL(wLabel,menuText,0,0,256,32,"Wolkenwelten");
	wid->flags |= WIDGET_BIG;
	widgetNewCPL(wLabel,menuText,0,32,256,32,(char *)VERSION);

	menuErrorLabel = widgetNewCPL(wLabel,menuText,1,-97,256,16,"");
	widgetNewCPL(wLabel,menuText,1,-33,256,16,menuTextInputLabel);

	initMainMenu();
	initSaveMenu();
	initServerMenu();
	initOptionsMenu();
}

static void drawMenuAttributions(){
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

void menuChangeFocus(int xoff,int yoff,bool ignoreOnTextInput){
	if(widgetFocused != NULL){
		if(ignoreOnTextInput && widgetFocused->type == wTextInput){return;}
		if((widgetFocused->type == wSlider) && (xoff != 0)){
			widgetFocused->vali = MAX(0,MIN(4096,(widgetFocused->vali + xoff*128)));
		}
		if(yoff < 0){
			if(!widgetEmit(widgetFocused,"selectNext")){
				widgetFocus(widgetNextSel(widgetFocused));
			}
		}else if(yoff > 0){
			if(!widgetEmit(widgetFocused,"selectPrev")){
				widgetFocus(widgetPrevSel(widgetFocused));
			}
		}
	} // No else if because then we immediatly focus on the widget on the other side
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
	//if(gameRunning){return;}
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
	if((saveMenu->gw > 0) || (serverMenu->gw > 0) || (optionsMenu->gw > 0)){
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
