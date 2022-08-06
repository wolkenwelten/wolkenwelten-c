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

#include "menu.h"

#include "../main.h"
#include "../nujel/widget.h"
#include "../gui/gui.h"
#include "../gui/repl.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../gui/menu/attribution.h"
#include "../gui/menu/mainmenu.h"
#include "../gfx/gl.h"
#include "../gfx/gfx.h"
#include "../gfx/shader.h"
#include "../gfx/texture.h"
#include "../gfx/textMesh.h"
#include "../misc/options.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/cto.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>
#include <string.h>

bool showAttribution  = false;
int  attributionLines = 0;

char menuTextInputLabel[32];

int  serverlistCount = 0;
char serverlistName[16][32];
char serverlistIP[16][64];

widget *menuBackground;

void menuChangeFocus(int xoff,int yoff,bool ignoreOnTextInput){
	if(widgetFocused != NULL){
		if(widgetFocused->type == wGameScreen){return;}
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
	if(widgetFocused == NULL){return;}
	if(widgetFocused->type == wGameScreen){return;}
	if(btn == 1){
		widgetEmit(widgetFocused,"altclick");
	}else{
		if(widgetFocused->type == wTextInput){
			widgetEmit(widgetFocused,"submit");
		}
		widgetEmit(widgetFocused,"click");
	}
}

bool menuCancel(){
	if((widgetFocused != NULL) && (widgetFocused->type == wGameScreen)){return false;}
	if(gameRunning){
		closeMainMenu();
		lispPanelClose();
		closeAttributions();
	} else {
		openMainMenu();
		lispPanelClose();
	}
	return true;
}

void menuSetError(const char *error){
	fprintf(stderr,"MenuError: %s\n",error);
	menuCloseGame();
}

void menuCloseGame(){
	openMainMenu();
}

void serverListAdd(const char *address, const char *name){
	if(serverlistCount >= 15){return;}
	snprintf(serverlistIP[serverlistCount],sizeof(serverlistIP[0]),"%s",address);
	snprintf(serverlistName[serverlistCount],sizeof(serverlistName[0]),"%s",name);
	serverlistIP[serverlistCount][sizeof(serverlistIP[0])-1]=0;
	serverlistName[serverlistCount][sizeof(serverlistName[0])-1]=0;
	serverlistCount++;
}
