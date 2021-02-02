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

int     lispInputCheckCountdown = -1;
lSymbol lispAutoCompleteList[32];
uint    lispAutoCompleteLen   = 0;
int     lispAutoCompleteStart = 0;
int     lispAutoCompleteEnd   = 0;
int     lispAutoCompleteSelection = -1;

void lispPanelOpen(){
	widgetSlideH(lispPanel, screenHeight-128);
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

void lispPanelShowReply(lVal *sym, const char *reply){
	int len = strnlen(reply,8192) + 1;
	if(lispLog == NULL){return;}
	for(int i=0;i<256;i++){
		const char *line = lispLog->valss[i];
		if(line == NULL){continue;}
		if(*line != ' '){continue;}
		for(;*line == ' ';line++){}
		if(strncmp(sym->vSymbol.c,line,6)){
			//printf("'%s' != '%s'\n",line,sym->vSymbol.c);
			continue;
		}
		free(lispLog->valss[i]);

		int soff,slen;
		for(soff = 0;isspace((unsigned char)reply[soff]) || (reply[soff] == '"');soff++){}
		for(slen = len-soff-1;isspace((unsigned char)reply[soff+slen-1]) || (reply[soff+slen-1] == '"');slen--){}
		lispLog->valss[i] = malloc(slen+3);
		for(int ii=0;ii<2;ii++){
			lispLog->valss[i][ii] = ' ';
		}
		memcpy(&lispLog->valss[i][2],&reply[soff],slen);
		lispLog->valss[i][slen+2] = 0;
		return;
	}
	fprintf(stderr,"Couldn't match SExpr Reply %s - %s\n",sym->vSymbol.c,reply);
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
	int l;
	if(lispInput->vals[0] == 0){return;}
	if((lispAutoCompleteLen > 0) && (lispAutoCompleteSelection >= 0)){
		strRemove(wid->vals,256,lispAutoCompleteStart,lispAutoCompleteEnd);
		strInsert(wid->vals,256,lispAutoCompleteStart,lispAutoCompleteList[lispAutoCompleteSelection].c);
		int slen = strnlen(lispAutoCompleteList[lispAutoCompleteSelection].c,16);
		const int off = slen - (textInputCursorPos - lispAutoCompleteStart);
		textInputCursorPos += off;
		textInputBufferLen += off;
		lispAutoCompleteLen = 0;
		lispAutoCompleteSelection = -1;
		return;
	}
	l = snprintf(buf,sizeof(buf)-1,"> %s",wid->vals);
	buf[l] = 0;
	widgetAddEntry(lispLog, buf);
	const char *result = lispEval(wid->vals);
	l = snprintf(buf,sizeof(buf)-1,"  %s",result);
	buf[l] = 0;
	widgetAddEntry(lispLog, buf);
	wid->vals[0] = 0;
	lispHistoryActive = -1;
	textInputFocus(wid);
}

void handlerLispSelectPrev(widget *wid){
	if(lispAutoCompleteLen > 0){
		if(++lispAutoCompleteSelection >= (int)lispAutoCompleteLen){lispAutoCompleteSelection = -1;}
		return;
	}
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

static lSymbol lispPanelGetPointSymbol(){
	lSymbol sym;
	sym.v[0] = sym.v[1] = 0;
	int i,m;
	const char *buf = widgetFocused->vals;
	if(buf == NULL){return sym;}
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
		const u8 c = buf[i];
		if(isspace(c))    {m--;break;}
		else if(c == '(') {m--;break;}
		else if(c == ')') {m--;break;}
		else if(c == '\''){m--;break;}
		else if(c == '"') {m--;break;}
	}
	if(m <= i){return sym;}
	lispAutoCompleteStart = i;
	lispAutoCompleteEnd = m;
	int len = MIN(15,m-i);
	memcpy(sym.v,&buf[i],len);
	return sym;
}

void lispPanelCheckAutoComplete(){
	static lSymbol lastSym = {0};
	if(widgetFocused == NULL){
		lispInputCheckCountdown = -1;
	}
	if(  lispInputCheckCountdown < 0){return;}
	if(--lispInputCheckCountdown >= 0){return;}
	lSymbol sym = lispPanelGetPointSymbol();
	if((sym.v[0] == lastSym.v[0]) && (sym.v[1] == lastSym.v[1])){
		return;
	}
	lispAutoCompleteSelection = -1;
	lispAutoCompleteLen = 0;
	if((sym.c[0] == 0) || (sym.c[1] == 0)){
		lastSym = sym;
		return;
	}

	lastSym = sym;
	lVal *newAC = lMatchClosureSym(clRoot,NULL,sym);
	for(lVal *n = newAC;(n != NULL) && (n->type == ltPair);n = n->vList.cdr){
		if((sym.v[0] == n->vList.car->vSymbol.v[0]) && (sym.v[1] == n->vList.car->vSymbol.v[1])){continue;}
		lispAutoCompleteList[lispAutoCompleteLen].v[0] = n->vList.car->vSymbol.v[0];
		lispAutoCompleteList[lispAutoCompleteLen].v[1] = n->vList.car->vSymbol.v[1];
		if(++lispAutoCompleteLen >= countof(lispAutoCompleteList)){break;}
	}
}
