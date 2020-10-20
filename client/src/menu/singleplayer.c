#define _GNU_SOURCE
#include "singleplayer.h"

#include "../main.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../misc/options.h"
#include "../menu/mainmenu.h"
#include "../../../common/src/tmp/cto.h"
#include "../../../common/src/misc/misc.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

widget *singleplayerMenu;
widget *saveList;
widget *newGame;
widget *newGameName;
widget *newGameSeed;

int  savegameCount = 0;
char savegameName[16][32];

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
	widgetLayVert(singleplayerMenu,16);
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

static void handlerLoadGame(widget *wid){
	loadSavegame(wid->vali);
}
static void handlerDeleteGame(widget *wid){
	deleteSavegame(wid->vali);
	checkSavegames();
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

static void handlerBackToMenu(widget *wid){
	(void)wid;
	openMainMenu();
}

void initSingleplayerMenu(){
	singleplayerMenu = widgetNewCP(wPanel,rootMenu,-1,0,0,-1);
	singleplayerMenu->flags |= WIDGET_HIDDEN;

	saveList = widgetNewCP(wSpace,singleplayerMenu,0,0,288,32);
	widgetNewCP(wHR,singleplayerMenu,16,0,256,32);
	widgetNewCPLH(wButton,singleplayerMenu,16,0,256,32,"New Game","click",handlerNewGame);
	widgetNewCPLH(wButton,singleplayerMenu,16,0,256,32,"Back to Menu","click",handlerBackToMenu);
	widgetLayVert(singleplayerMenu,16);

	newGame = widgetNewCP(wPanel,rootMenu,32,-1,288,0);
	newGame->flags |= WIDGET_HIDDEN;
	newGameName = widgetNewCPLH(wTextInput,newGame,16,16,256,32,"World Name","submit",handlerNewGameNext);
	newGameSeed = widgetNewCPLH(wTextInput,newGame,16,64,256,32,"World Seed","submit",handlerNewGameSubmit);
	widgetNewCPLH(wButton,newGame,16,112,120,32,"Cancel","click",handlerNewGameCancel);
	widgetNewCPLH(wButton,newGame,148,112,120,32,"Create","click",handlerNewGameSubmit);
	checkSavegames();
}

void openSingleplayerMenu(){
	closeAllMenus();
	checkSavegames();
	widgetSlideW(singleplayerMenu,288);
	widgetFocus(NULL);
}

void closeSingleplayerMenu(){
	widgetSlideW(singleplayerMenu, 0);
	widgetSlideH(newGame,          0);
}
