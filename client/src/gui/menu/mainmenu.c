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
#include "mainmenu.h"

#include "singleplayer.h"
#include "multiplayer.h"
#include "options.h"
#include "../gui.h"
#include "../menu.h"
#include "../textInput.h"
#include "../widget.h"
#include "../../main.h"
#include "../../misc/options.h"
#include "../../../../common/src/cto.h"
#include "../../../../common/src/misc/misc.h"

#include <stdio.h>
#include <string.h>

widget *mainMenu;

widget *buttonSP;
widget *buttonQuit;
widget *buttonReturnToGame;
widget *buttonExitToMenu;

static void focusMainMenu(){
	widgetFocus(gameRunning ? buttonReturnToGame : buttonSP);
}

static void handlerSingleplayer(widget *wid){
	(void)wid;
	openSingleplayerMenu();
}
static void handlerMultiplayer(widget *wid){
	(void)wid;
	openMultiplayerMenu();
}

static void handlerOptions(widget *wid){
	(void)wid;
	openOptionsMenu();

}
static void handlerAttribution(widget *wid){
	(void)wid;
	closeAllMenus();
	openAttributions();
}

static void handlerQuit(widget *wid){
	(void)wid;
	quit = true;
}

static void handlerReturnToGame(widget *wid){
	(void)wid;
	if(!gameRunning){return;}
	closeAllMenus();
	widgetFocus(widgetGameScreen);
}

static void handlerExitToMenu(widget *wid){
	(void)wid;
	if(!gameRunning){return;}
	menuCloseGame();
	openMainMenu();
	focusMainMenu();
}

void initMainMenu(){
	mainMenu = widgetNewCP(wPanel,rootMenu,rect(-1,0,0,-1));
	widgetNewCP  (wSpace ,mainMenu,rect(16,0,256,0));
	buttonSP = widgetNewCPLH(wButton,mainMenu,rect(16,0,256,32),"Singleplayer","click",handlerSingleplayer);
	widgetNewCPLH(wButton,mainMenu,rect(16,0,256,32),"Multiplayer","click",handlerMultiplayer);
	widgetNewCP  (wHorizontalRuler ,mainMenu,rect(16,0,256,32));
	widgetNewCPLH(wButton,mainMenu,rect(16,0,256,32),"Options","click",handlerOptions);
	widgetNewCPLH(wButton,mainMenu,rect(16,0,256,32),"Attribution","click",handlerAttribution);
	widgetNewCP  (wHorizontalRuler ,mainMenu,rect(16,0,256,32));
	buttonQuit         = widgetNewCPLH(wButton,mainMenu,rect(16,0,256,32),"Quit","click",handlerQuit);
	buttonExitToMenu   = widgetNewCPLH(wButton,mainMenu,rect(16,0,256,32),"Exit to Menu","click",handlerExitToMenu);
	buttonReturnToGame = widgetNewCPLH(wButton,mainMenu,rect(16,0,256,32),"Return to Game","click",handlerReturnToGame);

	widgetLayVert(mainMenu,16);
}

void openMainMenu(){
	closeAllMenus();
	if(gameRunning){
		buttonQuit->flags         |=  WIDGET_HIDDEN;
		buttonExitToMenu->flags   &= ~WIDGET_HIDDEN;
		buttonReturnToGame->flags &= ~WIDGET_HIDDEN;
	}else{
		buttonQuit->flags         &= ~WIDGET_HIDDEN;
		buttonExitToMenu->flags   |=  WIDGET_HIDDEN;
		buttonReturnToGame->flags |=  WIDGET_HIDDEN;
	}
	widgetSlideW(mainMenu, 288);
	mainMenu->goalArea.w = 288;
	focusMainMenu();
}

void closeMainMenu(){
	widgetSlideW(mainMenu, 0);
}
