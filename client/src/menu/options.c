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

#include "options.h"

#include "../main.h"
#include "../gfx/gfx.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../misc/options.h"
#include "../menu/mainmenu.h"
#include "../../../common/src/tmp/cto.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>
#include <string.h>

widget *optionsMenu;

widget *optionsName;
widget *optionsVolume;
widget *optionsRenderDistance;
widget *optionsMouseSensitivity;

float oldRenderDistance   = 0.f;
float oldMouseSensitivity = 0.f;

static void focusOptions(){
	widgetFocus(optionsName);
}

static void handlerRenderDistanceChanged(widget *wid){
	setRenderDistance((wid->vali / 4096.f) * (512-64) + 64.f);
}

static void handlerMouseSensitivityChanged(widget *wid){
	optionMouseSensitivy = (wid->vali / 4096.f)+0.01f;
}

static void handlerOptionsSave(widget *wid){
	(void)wid;
	optionSoundVolume = optionsVolume->vali / 4096.f;
	setRenderDistance((optionsRenderDistance->vali / 4096.f) * (512-64) + 64.f);
	oldRenderDistance = renderDistance;
	snprintf(playerName,sizeof(playerName),"%s",optionsName->vals);
	saveOptions();
	openMainMenu();
}

static void handlerOptionsCancel(widget *wid){
	(void)wid;
	optionsVolume->vali = optionSoundVolume * 4096.f;
	snprintf(optionsName->vals,256,"%s",playerName);
	setRenderDistance(oldRenderDistance);
	openMainMenu();
}

void initOptionsMenu(){
	optionsMenu = widgetNewCP(wPanel,rootMenu,-1,0,0,-1);
	optionsMenu->flags |= WIDGET_HIDDEN;

	widgetNewCP  (wSpace ,optionsMenu,16,0,256,0);

	optionsName = widgetNewCPL(wTextInput,optionsMenu,16,0,256,32,"Playername");
	strncpy(optionsName->vals,playerName,256);
	optionsVolume = widgetNewCPL(wSlider,optionsMenu,16,0,256,32,"Volume");
	optionsVolume->vali = optionSoundVolume * 4096.f;

	optionsRenderDistance = widgetNewCPL(wSlider,optionsMenu,16,0,256,32,"Render Distance");
	optionsRenderDistance->vali = ((renderDistance-64.f) / (512-64)) * 4096.f;
	widgetBind(optionsRenderDistance,"change",handlerRenderDistanceChanged);

	optionsMouseSensitivity = widgetNewCPL(wSlider,optionsMenu,16,0,256,32,"Mouse Sensitivy");
	optionsMouseSensitivity->vali = (optionMouseSensitivy-0.01f) * 4096.f;
	widgetBind(optionsMouseSensitivity,"change",handlerMouseSensitivityChanged);

	widgetNewCP  (wHR ,optionsMenu,16,0,256,32);
	widgetNewCPLH(wButton,optionsMenu,16,0,256,32,"Save","click",handlerOptionsSave);
	widgetNewCPLH(wButton,optionsMenu,16,0,256,32,"Cancel","click",handlerOptionsCancel);
	widgetLayVert(optionsMenu,16);
}

void openOptionsMenu(){
	closeAllMenus();
	widgetSlideW(optionsMenu,288);
	focusOptions();
	oldRenderDistance = renderDistance;
	oldMouseSensitivity = optionMouseSensitivy;
	optionsRenderDistance->vali = ((renderDistance-64.f) / (512-64)) * 4096.f;
	optionsMouseSensitivity->vali = (optionMouseSensitivy-0.01f) * 4096.f;
}

void closeOptionsMenu(){
	widgetSlideW(optionsMenu,0);
}
