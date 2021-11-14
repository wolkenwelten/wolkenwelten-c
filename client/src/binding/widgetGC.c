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
#include "widgetGC.h"

#include "../main.h"
#include "../binding/widget.h"
#include "../gui/widget.h"
#include "../gui/gui.h"

#include "../../../common/nujel/lib/api.h"
#include "../../../common/nujel/lib/nujel.h"
#include "../../../common/nujel/lib/exception.h"
#include "../../../common/nujel/lib/allocation/garbage-collection.h"
#include "../../../common/nujel/lib/allocation/roots.h"
#include "../../../common/nujel/lib/s-expression/writer.h"

#include <string.h>

u8 lWidMarkMap    [WID_MAX];
u8 lEvhMarkMap    [EVH_MAX];

void (*rootsMarkerChainNext)() = NULL;
void (*sweeperChainNext)() = NULL;

static int widgetID(const widget *w){
	return w - widgetList;
}

static int eventHandlerID(const eventHandler *evh){
	return evh - eventHandlerList;
}

static eventHandler *eventHandlerGet(int i){
	return &eventHandlerList[i];
}

static void eventHandlerMark(const eventHandler *evh){
	if(evh == NULL){return;}
	const int i = eventHandlerID(evh);
	if(lEvhMarkMap[i]){return;}
	lEvhMarkMap[i] = 1;
	if(evh->lisp){
		lValGCMark(evh->lispHandler);
	}
	eventHandlerMark(evh->next);
}

void widgetMark(const widget *w){
	if((w == NULL) || lWidMarkMap[widgetID(w)]){return;}
	lWidMarkMap[widgetID(w)] = 1;
	widgetMark(w->parent);
	widgetMark(w->child);
	widgetMark(w->prev);
	widgetMark(w->next);
	eventHandlerMark(w->firstHandler);
}

void lWidgetMarkI(uint i){
	widgetMark(widgetGet(i));
}

static void widgetMarker(){
	widgetMark(rootMenu);
	widgetMark(widgetGameScreen);
	widgetMark(widgetFocused);
	widgetMark(widgetFirstFree);

	if(rootsMarkerChainNext){rootsMarkerChainNext();}
}

static void widgetSweeper(){
	for(int i=0;i<widgetMax;i++){
		if(lWidMarkMap[i]){
			lWidMarkMap[i] = 0;
		}else{
			widgetFree(widgetGet(i));
		}
	}

	for(int i=0;i<eventHandlerMax;i++){
		if(lEvhMarkMap[i]){
			lEvhMarkMap[i] = 0;
		}else{
			eventHandlerFree(eventHandlerGet(i));
		}
	}
	if(sweeperChainNext){sweeperChainNext();}
}

void widgetGCInit(){
	sweeperChainNext = sweeperChain;
	sweeperChain = widgetSweeper;
	rootsMarkerChainNext = rootsMarkerChain;
	rootsMarkerChain = widgetMarker;
}
