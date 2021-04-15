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

#include "multiplayer.h"

#include "../main.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../misc/options.h"
#include "../menu/mainmenu.h"
#include "../../../common/src/cto.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>
#include <string.h>

widget *multiplayerMenu;
widget *serverList;
widget *newServer;
widget *newServerName;
widget *newServerIP;
widget *firstServer = NULL;
widget *buttonNewServer;

static void focusMultiPlayer(){
	widgetFocus(firstServer != NULL ? firstServer : buttonNewServer);
}


static void handlerJoinServer(widget *wid);
static void handlerDeleteServer(widget *wid);
static void refreshServerList(){
	widgetEmpty(serverList);
	firstServer = NULL;
	for(int i=0;i<serverlistCount;i++){
		widget *button = widgetNewCPL(wButtonDel,serverList,16,i*48,256,32,serverlistName[i]);
		widgetBind(button,"click",handlerJoinServer);
		widgetBind(button,"altclick",handlerDeleteServer);
		button->vali = i;
		firstServer = button;
	}

	serverList->h = serverlistCount * 48;
	widgetLayVert(multiplayerMenu,16);
	focusMultiPlayer();
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
	menuCloseGame();
	snprintf(serverName,sizeof(serverName),"%.63s",serverlistIP[i]);
	startMultiplayer();
}
static void handlerJoinServer(widget *wid){
	joinServer(wid->vali);
}
static void handlerDeleteServer(widget *wid){
	delServer(wid->vali);
	refreshServerList();
	saveOptions();
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
	focusMultiPlayer();
}
static void handlerNewServerNext(widget *wid){
	(void)wid;
	widgetFocus(newServerIP);
}

static void handlerBackToMenu(widget *wid){
	(void)wid;
	openMainMenu();
}

void initMultiplayerMenu(){
	multiplayerMenu = widgetNewCP(wPanel,rootMenu,-1,0,0,-1);
	multiplayerMenu->flags |= WIDGET_HIDDEN;

	serverList = widgetNewCP(wSpace,multiplayerMenu,0,0,288,32);
	widgetNewCP(wHR,multiplayerMenu,16,0,256,32);
	buttonNewServer = widgetNewCPLH(wButton,multiplayerMenu,16,0,256,32,"New Server","click",handlerNewServer);
	widgetNewCPLH(wButton,multiplayerMenu,16,0,256,32,"Back to Menu","click",handlerBackToMenu);
	widgetLayVert(multiplayerMenu,16);

	newServer = widgetNewCP(wPanel,rootMenu,32,-1,288,0);
	newServer->flags |= WIDGET_HIDDEN;
	newServerName = widgetNewCPLH(wTextInput,newServer,16,16,256,32,"Server Name","submit",handlerNewServerNext);
	newServerIP = widgetNewCPLH(wTextInput,newServer,16,64,256,32,"IP / Domain","submit",handlerNewServerSubmit);
	widgetNewCPLH(wButton,newServer,16,112,120,32,"Cancel","click",handlerNewServerCancel);
	widgetNewCPLH(wButton,newServer,148,112,120,32,"Create","click",handlerNewServerSubmit);
	checkServers();
}

void openMultiplayerMenu(){
	closeAllMenus();
	checkServers();
	widgetSlideW(multiplayerMenu,288);
	focusMultiPlayer();
}

void closeMultiplayerMenu(){
	widgetSlideW(multiplayerMenu, 0);
	widgetSlideH(newServer,  0);
}
