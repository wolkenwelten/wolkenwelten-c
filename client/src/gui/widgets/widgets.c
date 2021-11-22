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
#include "widgets.h"

#include "../../game/recipe.h"
#include "../../gfx/gfx.h"
#include "../../gfx/textMesh.h"
#include "../../../../common/src/game/item.h"

#include <string.h>

typedef struct {
	box2D area;
	const widget *wid;
} popupQueueEntry;

popupQueueEntry popupQueue[16];
uint popupQueueLength = 0;


void widgetDrawTextLog(const widget *wid, textMesh *m, box2D area);
void widgetDrawTextScroller(const widget *wid, textMesh *m, box2D area);
void widgetDrawRecipeInfo(const widget *wid, textMesh *m, box2D area);
void widgetDrawRecipeSlot(const widget *wid, textMesh *m, box2D area);
void widgetDrawItemSlot(const widget *wid, textMesh *m, box2D area, const item *itm);
void widgetDrawSlider(const widget *wid, textMesh *m, box2D area);
void widgetDrawTextInput(const widget *wid, textMesh *m, box2D area);
void widgetDrawBackground(const widget *wid, textMesh *m, box2D area);
void widgetDrawRadioButton(const widget *wid, textMesh *m, box2D area);
void widgetDrawButtondel(const widget *wid, textMesh *m, box2D area);
void widgetDrawButton(const widget *wid, textMesh *m, box2D area);
void widgetDrawLispLine(textMesh *m, box2D area, int size, const char *rawLine, int lambda, int mark, int cursor);
void widgetDrawHorizontalRuler(const widget *wid, textMesh *m, box2D area);
void widgetDrawPanel(const widget *wid, textMesh *m, box2D area);
void widgetDrawLabel(const widget *wid, textMesh *m, box2D area);

static void widgetDrawPopupItemSlot(textMesh *m,const item *itm, box2D area){
	if(itm == NULL){return;}
	const char *name = itemGetName(itm);
	if(name == NULL){return;}
	uint len = strnlen(name,256);

	u32 bcolor = 0xCC444444;
	u32 tcolor = 0xCC111111;
	int xoff   = (len * 8)+8;
	int yoff   = MAX(0,MIN((int)(area.y-32),(int)(screenHeight-32)));
	int width  = xoff * 2;
	xoff = MAX(0,MIN((int)(area.x+(area.w/2)-xoff),(int)(screenWidth-width)));
	textMeshVGradient(m,xoff,yoff,width,32,tcolor,bcolor);
	textMeshAddStrPS(m,xoff+8,yoff+8,2,name);
}

void widgetAddPopup(const widget *wid, box2D area){
	if(popupQueueLength >= 16){return;}
	popupQueue[popupQueueLength++] = (popupQueueEntry){area,wid};
}

void widgetDrawPopups(textMesh *m){
	for(uint i=0;i<popupQueueLength;i++){
		popupQueueEntry *qe = &popupQueue[i];
		if(qe->wid == NULL){continue;}
		switch(qe->wid->type){
		default:
			break;
		case wRecipeSlot: {
			const item recipeRes = recipeGetResult(qe->wid->valu);
			widgetDrawPopupItemSlot(m,&recipeRes,qe->area);
			break; }
		case wItem: {
			const item *itm = &qe->wid->valItem;
			widgetDrawPopupItemSlot(m,itm,qe->area);
			break; }
		case wItemSlot: {
			const item *itm = qe->wid->valItemSlot;
			widgetDrawPopupItemSlot(m,itm,qe->area);
			break; }
		}
	}
	popupQueueLength = 0;
}

void widgetDrawSingle(const widget *wid, textMesh *m, box2D area){
	if((wid == NULL) || (area.x >= screenWidth) || (area.y >= screenHeight) || ((area.x + area.w) <= 0) || ((area.y + area.h) <= 0)){return;}

	switch(wid->type){
	case wNone:
	case wSpace:
	case wGameScreen:
		break;
	case wPanel:
		widgetDrawPanel(wid,m,area);
		break;
	case wBackground:
		widgetDrawBackground(wid,m,area);
		break;
	case wHorizontalRuler:
		widgetDrawHorizontalRuler(wid,m,area);
		break;
	case wLabel:
		widgetDrawLabel(wid,m,area);
		break;
	case wTextInput:
		widgetDrawTextInput(wid,m,area);
		break;
	case wButton:
		widgetDrawButton(wid,m,area);
		break;
	case wButtonDel:
		widgetDrawButtondel(wid,m,area);
		break;
	case wRadioButton:
		widgetDrawRadioButton(wid,m,area);
		break;
	case wSlider:
		widgetDrawSlider(wid,m,area);
		break;
	case wItem:
		widgetDrawItemSlot(wid,m,area,&wid->valItem);
		break;
	case wItemSlot:
		widgetDrawItemSlot(wid,m,area,wid->valItemSlot);
		break;
	case wRecipeSlot:
		widgetDrawRecipeSlot(wid,m,area);
		break;
	case wRecipeInfo:
		widgetDrawRecipeInfo(wid,m,area);
		break;
	case wTextScroller:
		widgetDrawTextScroller(wid,m,area);
		break;
	case wTextLog:
		widgetDrawTextLog(wid,m,area);
		break;
	}
}
