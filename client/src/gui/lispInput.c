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

#include "lispInput.h"

#include "../main.h"
#include "../gfx/gfx.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../misc/lisp.h"
#include "../../../common/src/misc/misc.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

widget *lispPanel;
widget *lispLog;
widget *lispInput;
int     lispHistoryActive = -1;
bool    lispPanelVisible  = false;

int      lispInputCheckCountdown = -1;
lSymbol *lispAutoCompleteList[32];
uint     lispAutoCompleteLen   = 0;
int      lispAutoCompleteStart = 0;
int      lispAutoCompleteEnd   = 0;
int      lispAutoCompleteSelection = -1;
bool     lispAutoCompleteCompleteSymbol = false;

char     lispAutoCompleteDescription[256];
lSymbol *lispAutoCompleteDescriptionSymbol;

void lispPanelOpen(){
	widgetSlideH(lispPanel, screenHeight-156);
	widgetFocus(lispInput);
	lispPanelVisible = true;
}

void lispPanelClose(){
	widgetSlideH(lispPanel, 0);
	if(gameRunning){
		widgetFocus(widgetGameScreen);
	}
	lispPanelVisible = false;
}

void lispPanelShowReply(const char *reply){
	int len = strnlen(reply,8192) + 1;
	if(lispLog == NULL){return;}
	for(int i=0;i<256;i++){
		const char *line = lispLog->valss[i];
		if(line == NULL){continue;}
		if(*line != ' '){continue;}
		for(;*line == ' ';line++){}
		free(lispLog->valss[i]);

		int soff,slen;
		for(soff = 0;isspace((u8)reply[soff]) || (reply[soff] == '"');soff++){}
		for(slen = len-soff-1;isspace((u8)reply[soff+slen-1]) || (reply[soff+slen-1] == '"');slen--){}
		lispLog->valss[i] = malloc(slen+3);
		for(int ii=0;ii<2;ii++){
			lispLog->valss[i][ii] = ' ';
		}
		memcpy(&lispLog->valss[i][2],&reply[soff],MINMAX(slen,0,256));
		lispLog->valss[i][slen+2] = 0;
		return;
	}
}

void lispPanelToggle(){
	if(lispPanelVisible){
		lispPanelOpen();
	}else{
		lispPanelClose();
	}
}

void handlerLispSubmit(widget *wid){
	char buf[8192];
	if(lispInput->vals[0] == 0){return;}
	if((lispAutoCompleteLen > 0) && (lispAutoCompleteSelection >= 0)){
		strRemove(wid->vals,256,lispAutoCompleteStart,lispAutoCompleteEnd);
		strInsert(wid->vals,256,lispAutoCompleteStart,lispAutoCompleteList[lispAutoCompleteSelection]->c);
		int slen = strnlen(lispAutoCompleteList[lispAutoCompleteSelection]->c,sizeof(lSymbol));
		const int off = slen - (textInputCursorPos - lispAutoCompleteStart);
		textInputCursorPos += off;
		textInputBufferLen += off;
		lispAutoCompleteLen = 0;
		lispAutoCompleteSelection = -1;
		return;
	}else{
		snprintf(buf,sizeof(buf),"> %s",wid->vals);
		widgetAddEntry(lispLog, buf);
		const char *result = lispEval(wid->vals);
		snprintf(buf,sizeof(buf),"  %s",result);
		widgetAddEntry(lispLog, buf);
	}
	lispInput->vals[0] = 0;
	lispHistoryActive = -1;
	textInputFocus(wid);
}

void handlerLispSelectPrev(widget *wid){
	if(lispAutoCompleteLen > 0){
		if(++lispAutoCompleteSelection >= (int)lispAutoCompleteLen){lispAutoCompleteSelection = -1;}
		return;
	}
	if(lispHistoryActive+1 > 255){return;}
	const char *msg = lispLog->valss[lispHistoryActive+1];
	if(msg == NULL){return;}
	if(*msg == 0)  {return;}
	++lispHistoryActive;
	if(*msg == ' '){
		handlerLispSelectPrev(wid);
		return;
	}
	int firstSpace;
	for(firstSpace=0;firstSpace<255;firstSpace++){
		if(msg[firstSpace] == ' '){break;}
	}
	firstSpace++;
	memcpy(wid->vals,&msg[firstSpace],255-firstSpace);
	wid->vals[255]=0;
	textInputFocus(wid);
}

void handlerLispSelectNext(widget *wid){
	if(lispAutoCompleteLen > 0){
		if(--lispAutoCompleteSelection < -1){lispAutoCompleteSelection = lispAutoCompleteLen-1;}
		return;
	}
	if(lispHistoryActive < 0){return;}
	const char *msg = lispLog->valss[lispHistoryActive-1];
	if(lispHistoryActive <= 0){
		memset(wid->vals,0,256);
		textInputFocus(wid);
		return;
	}
	if(msg == NULL){return;}
	if(*msg == 0)  {return;}
	--lispHistoryActive;
	if(*msg == ' '){
		handlerLispSelectNext(wid);
		return;
	}
	int firstSpace;
	for(firstSpace=0;firstSpace<255;firstSpace++){
		if(wid->vals[firstSpace] == ' '){break;}
	}
	memcpy(wid->vals,&msg[firstSpace],255-firstSpace);
	wid->vals[255]=0;
	textInputFocus(wid);
}

void lispInputInit(){
	lispPanel = widgetNewCP(wPanel,rootMenu,64,0,screenWidth-128,0);
	lispPanel->flags |= WIDGET_HIDDEN;
	lispLog = widgetNewCP(wTextLog,lispPanel,0,0,-1,-32);
	lispLog->flags |= WIDGET_LISP;
	lispInput = widgetNewCP(wTextInput,lispPanel,0,-1,-1,32);
	lispInput->flags |= WIDGET_LISP;
	widgetBind(lispInput,"submit",handlerLispSubmit);
	widgetBind(lispInput,"selectPrev",handlerLispSelectPrev);
	widgetBind(lispInput,"selectNext",handlerLispSelectNext);
}

void lispPanelGetPointSymbol(){
	int i,m;
	lispAutoCompleteStart = lispAutoCompleteEnd = 0;
	const char *buf = widgetFocused->vals;
	if(buf == NULL){return;}
	for(i = textInputCursorPos; i >= 0; i--){
		const u8 c = buf[i];
		if(isspace(c))    {i++;break;}
		else if(c == '(') {i++;break;}
		else if(c == ')') {i++;break;}
		else if(c == '\''){i++;break;}
		else if(c == '"') {i++;break;}
	}
	i = MAX(0,i);
	for(m = i; buf[m] != 0; m++){
		const u8 c = buf[m];
		if(isspace(c))    {m--;break;}
		else if(c == '(') {m--;break;}
		else if(c == ')') {m--;break;}
		else if(c == '\''){m--;break;}
		else if(c == '"') {m--;break;}
	}
	if(m <= i){return;}
	lispAutoCompleteStart = i;
	lispAutoCompleteEnd = m;
}

void lispPanelCheckAutoComplete(){
	static u64 lastSel = 0;
	if(widgetFocused == NULL){
		lispInputCheckCountdown = -1;
	}
	if(  lispInputCheckCountdown < 0){return;}
	if(--lispInputCheckCountdown >= 0){return;}
	lispPanelGetPointSymbol();
	if(lispAutoCompleteEnd <= lispAutoCompleteStart){return;}
	const u64 newSel = lispAutoCompleteEnd ^ lispAutoCompleteStart;
	if(newSel == lastSel){return;}
	lispAutoCompleteCompleteSymbol = false;
	lastSel = newSel;
	lispAutoCompleteSelection = -1;
	lispAutoCompleteLen = 0;
	lVal *newAC = lSearchClosureSym(lCloI(clRoot),NULL,&widgetFocused->vals[lispAutoCompleteStart],lispAutoCompleteEnd - lispAutoCompleteStart);
	for(lVal *n = newAC;(n != NULL) && (n->type == ltPair);n = n->vList.cdr){
		lispAutoCompleteList[lispAutoCompleteLen] = lGetSymbol(lCar(n));
		if(++lispAutoCompleteLen >= countof(lispAutoCompleteList)){break;}
	}
	if(lispAutoCompleteLen == 1){lispAutoCompleteSelection = 0;}
	if((int)strnlen(lGetSymbol(lCar(newAC))->c,sizeof(lSymbol)) == (lispAutoCompleteEnd - lispAutoCompleteStart)){
		lispAutoCompleteCompleteSymbol = true;
	}
}

void lispPanelCheckAutoCompleteDescription(){
	if((widgetFocused == NULL) || (lispAutoCompleteLen == 0) || (lispAutoCompleteSelection < 0)){
		lispAutoCompleteDescription[0] = 0;
		lispAutoCompleteDescriptionSymbol = NULL;
		return;
	}
	if(lispAutoCompleteList[lispAutoCompleteSelection] == lispAutoCompleteDescriptionSymbol){
		return;
	}
	lispAutoCompleteDescriptionSymbol = lispAutoCompleteList[lispAutoCompleteSelection];

	char sBuf[128];
	snprintf(sBuf,sizeof(sBuf),"(describe \"%s\")",lispAutoCompleteDescriptionSymbol->c);
	const char *str = lispEval(sBuf);
	snprintf(lispAutoCompleteDescription,sizeof(lispAutoCompleteDescription),"%s",str);
	lispAutoCompleteDescription[sizeof(lispAutoCompleteDescription)-1]=0;
}
