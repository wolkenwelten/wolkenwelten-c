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

float oldRenderDistance = 0.f;

static void handlerRenderDistanceChanged(widget *wid){
	setRenderDistance((wid->vali / 4096.f) * (512-64) + 64.f);
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

	widgetNewCP  (wHR ,optionsMenu,16,0,256,32);
	widgetNewCPLH(wButton,optionsMenu,16,0,256,32,"Save","click",handlerOptionsSave);
	widgetNewCPLH(wButton,optionsMenu,16,0,256,32,"Cancel","click",handlerOptionsCancel);
	widgetLayVert(optionsMenu,16);
}

void openOptionsMenu(){
	closeAllMenus();
	widgetSlideW(optionsMenu,288);
	widgetFocus(NULL);
	oldRenderDistance = renderDistance;
	optionsRenderDistance->vali = ((renderDistance-64.f) / (512-64)) * 4096.f;
}

void closeOptionsMenu(){
	widgetSlideW(optionsMenu,0);
}
