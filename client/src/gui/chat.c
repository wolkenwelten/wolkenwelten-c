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

#include "chat.h"

#include "../main.h"
#include "../gfx/gfx.h"
#include "../gfx/textMesh.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../network/chat.h"
#include "../sdl/sdl.h"

#include <string.h>

widget *chatPanel;
widget *chatText;

void chatOpen(){
	if(gameControlsInactive()){return;}
	widgetSlideH(chatPanel, 64);
	widgetFocus(chatText);
}

void chatClose(){
	chatText->vals[0]  = 0;
	widgetSlideH(chatPanel, 0);
}

static void handlerChatChange(widget *wid){
	if(wid == NULL){return;}
	if(wid->vals == NULL){return;}
	if(wid->vals[0] == '.'){
		wid->flags |= WIDGET_LISP_SYNTAX_HIGHLIGHT;
	}else{
		wid->flags &= ~WIDGET_LISP_SYNTAX_HIGHLIGHT;
	}
}

void handlerChatSubmit(widget *wid){
	if(chatText->vals[0] != 0){
		msgSendChatMessage(chatText->vals);
		chatResetHistorySel();
	}
	handlerRootHud(wid);
	wid->flags &= ~WIDGET_LISP_SYNTAX_HIGHLIGHT;
}

void handlerChatSelectPrev(widget *wid){
	const char *msg = chatGetPrevHistory();
	if(*msg == 0){return;}
	memcpy(wid->vals,msg,256);
	textInputFocus(wid);
	handlerChatChange(wid);
}

void handlerChatSelectNext(widget *wid){
	const char *msg = chatGetNextHistory();
	if(*msg == 0){return;}
	memcpy(wid->vals,msg,256);
	textInputFocus(wid);
	handlerChatChange(wid);
}

void handlerChatBlur(widget *wid){
	chatResetHistorySel();
	wid->flags &= ~WIDGET_LISP_SYNTAX_HIGHLIGHT;
}

void chatInit(){
	chatText  = widgetNewCPLH(wTextInput,chatPanel,16,16,440,32,"Chat Message","submit",handlerChatSubmit);
	widgetBind(chatText,"change",handlerChatChange);
	widgetBind(chatText,"blur",handlerChatBlur);
	widgetBind(chatText,"selectPrev",handlerChatSelectPrev);
	widgetBind(chatText,"selectNext",handlerChatSelectNext);
	widgetNewCPLH(wButton,chatPanel,-16,16,24,32,"\xA8","click",handlerChatSubmit);
}

void chatDraw(textMesh *guim){
	guim->sy   = screenHeight - (9*16) - (chatPanel->h-8);
	guim->sx   = 24;
	guim->size = 1;
	guim->font = 1;
	for(int i=0;i<8;i++){
		textMeshAddString(guim,chatLog[i]);
		guim->sy += 8;
	}
	guim->size = 2;
	for(int i=8;i<12;i++){
		textMeshAddString(guim,chatLog[i]);
		guim->sy += 16;
	}
	guim->font = 0;
}
