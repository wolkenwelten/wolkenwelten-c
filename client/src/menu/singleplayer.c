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

#include "singleplayer.h"

#include "../main.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../misc/options.h"
#include "../menu/mainmenu.h"
#include "../../../common/src/cto.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/sha1.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

widget *singleplayerMenu;
widget *saveList;
widget *newGame;
widget *newGameName;
widget *newGameSeed;
widget *firstSave;
widget *buttonNewGame;

int  savegameCount = 0;
char savegameName[16][32];

static void focusSinglePlayer(){
	widgetFocus(firstSave != NULL ? firstSave : buttonNewGame);
}

static void handlerLoadGame(widget *wid);
static void handlerDeleteGame(widget *wid);
static void refreshSaveList(){
	widgetEmpty(saveList);
	firstSave = NULL;
	for(int i=0;i<savegameCount;i++){
		widget *button = widgetNewCPL(wButtonDel,saveList,16,i*48,256,32,savegameName[i]);
		widgetBind(button,"click",handlerLoadGame);
		widgetBind(button,"altclick",handlerDeleteGame);
		button->vali = i;
		if(firstSave == NULL){firstSave = button;}
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
		if(++savegameCount >= 16){break;}
	}
	closedir(dp);
	refreshSaveList();
	focusSinglePlayer();
}

static void loadSavegame(int i){
	if(i < 0){return;}
	if(i >= savegameCount){return;}
	menuCloseGame();
	snprintf(optionSavegame,sizeof(optionSavegame),"%s",savegameName[i]);
	startSingleplayer();
}

static void deleteSavegame(int i){
	static char buf[64];
	if(i < 0)             {return;}
	if(i > savegameCount) {return;}
	snprintf(buf,sizeof(buf),"save/%s",savegameName[i]);
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
		optionWorldSeed = SHA1Simple(newGameSeed->vals,strnlen(newGameSeed->vals,256));
	}
	handlerNewGameCancel(wid);
	startSingleplayer();
}

static void handlerNewGameNext(widget *wid){
	(void)wid;
	if(wid == newGameName){
		if(newGameName->vals[0] == 0){snprintf(newGameName->vals,256,"New Game");}
		widgetFocus(newGameSeed);
	}else if(wid == newGameSeed){
		handlerNewGameSubmit(wid);
	}
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
	buttonNewGame = widgetNewCPLH(wButton,singleplayerMenu,16,0,256,32,"New Game","click",handlerNewGame);
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
}

void closeSingleplayerMenu(){
	widgetSlideW(singleplayerMenu, 0);
	widgetSlideH(newGame,          0);
}
