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

#include "textInput.h"
#include "../gui/lispInput.h"
#include "../sdl/input/keyboard.h"
#include "../sdl/sdl.h"
#include "../gfx/textMesh.h"
#include "../gui/widget.h"

#include <stdio.h>
#include <string.h>
#include <SDL.h>

int  textInputMark      = -1;
int  textInputBufferLen = 0;
int  textInputCursorPos = 0;
bool textInputStarted   = false;

void textInputFocus(widget *wid){
	textInputMark = -1;
	if(wid->type == wTextInput){
		textInputBufferLen = strnlen(wid->vals,255);
	}
	textInputCursorPos = textInputBufferLen;

	if(textInputStarted){return;}
	SDL_StartTextInput();
	textInputStarted = true;
}

void textInputBlur(widget *wid){
	(void)wid;
	textInputBufferLen = 0;
	textInputCursorPos = 0;
	if(!textInputStarted){return;}
	SDL_StopTextInput();
	textInputStarted = false;
}

int textInputActive(){
	if(widgetFocused == NULL){return 0;}
	if(widgetFocused->type == wTextInput){return 1;}
	return 0;
}

void textInputEnter(){
	if(!textInputActive()){return;}
	widgetEmit(widgetFocused,"submit");
}

void textInputDelSelection(){
	if(!textInputActive()){return;}
	if(textInputMark < 0){return;}
	char *textInputBuffer = widgetFocused->vals;
	int sMin = MIN(textInputMark,textInputCursorPos);
	int sMax = MAX(textInputMark,textInputCursorPos);
	int len = sMax - sMin;
	if(len <= 0){return;}

	memmove(&textInputBuffer[sMin],&textInputBuffer[sMax],textInputBufferLen - sMax);
	textInputBufferLen -= len;
	textInputBuffer[textInputBufferLen] = 0;
	textInputCursorPos = sMin;
	textInputMark = -1;
}

void textInputBackspace(int moveForward){
	if(!textInputActive()){return;}
	if(textInputMark >= 0){return textInputDelSelection();}
	if((textInputCursorPos + moveForward) == 0){return;}
	if((textInputCursorPos + moveForward) > textInputBufferLen){return;}
	char *textInputBuffer = widgetFocused->vals;
	textInputCursorPos += moveForward;
	for(int i=textInputCursorPos-1;i<textInputBufferLen-1;i++){
		textInputBuffer[i] = textInputBuffer[i+1];
	}
	--textInputCursorPos;
	--textInputBufferLen;
	textInputBuffer[textInputBufferLen] = 0;
	widgetEmit(widgetFocused,"change");
}

void textInputAppend(const char *s){
	int slen = strnlen(s,256);
	if(!textInputActive()){return;}
	if((widgetFocused->parent->flags & WIDGET_ANIMATE) && (s[0] == '`' && s[1] == 0)){
		return;
	}
	if(textInputMark >= 0){return textInputDelSelection();}
	char *textInputBuffer = widgetFocused->vals;
	if((slen + textInputBufferLen) > 255){slen = 255-textInputBufferLen;}
	if(textInputCursorPos != textInputBufferLen){
		for(int i=textInputBufferLen-1;i>=textInputCursorPos;i--){
			textInputBuffer[i+slen] = textInputBuffer[i];
		}
	}
	strncpy(textInputBuffer+textInputCursorPos,s,slen);
	textInputCursorPos += slen;
	textInputBufferLen += slen;
	if(textInputCursorPos >= 255){ textInputCursorPos = 255; }
	if(textInputBufferLen >= 255){ textInputBufferLen = 255; }
	textInputBuffer[textInputBufferLen] = 0;
	widgetEmit(widgetFocused,"change");
}

void textInputCopy(){
	if(!textInputActive()){return;}
	char *textInputBuffer = widgetFocused->vals;
	int sMin = MIN(textInputMark,textInputCursorPos);
	int sMax = MAX(textInputMark,textInputCursorPos);
	int len  = sMax - sMin;
	if(len <= 0){return;}
	char *buf = malloc(len+1);
	memcpy(buf,&textInputBuffer[sMin],len);
	buf[len]=0;
	SDL_SetClipboardText(buf);
	free(buf);
}

void textInputCut(){
	textInputCopy();
	textInputBackspace(0);
}

void textInputPaste(){
	if(!textInputActive()){return;}
	if(!SDL_HasClipboardText()){return;}
	char *text = SDL_GetClipboardText();
	if(text == NULL){return;}
	textInputAppend(text);
	SDL_free(text);
}

void textInputHome(){
	char *textInputBuffer = widgetFocused->vals;
	char *c = &textInputBuffer[0];
	for(; isspace((u8)*c); c++){}
	const int indentPos = c - textInputBuffer;
	if(textInputCursorPos <= indentPos){
		textInputCursorPos = 0;
	}else{
		textInputCursorPos = indentPos;
	}
}

void textInputEnd(){
	char *textInputBuffer = widgetFocused->vals;
	char *c = &textInputBuffer[textInputBufferLen-1];
	for(; isspace((u8)*c); c--){}
	const int indentPos = c - textInputBuffer + 1;
	if(textInputCursorPos >= indentPos){
		textInputCursorPos = textInputBufferLen;;
	}else{
		textInputCursorPos = indentPos;
	}
}

void textInputCheckMark(const SDL_Event *e){
	if(e->key.keysym.mod & KMOD_SHIFT){
		if(textInputMark < 0){
			textInputMark = textInputCursorPos;
		}
	}else{
		textInputMark = -1;
	}
}

bool textInputEvent(const SDL_Event *e){
	if(!textInputActive()){return false;}
	if(!textInputStarted){textInputFocus(widgetFocused);}

	switch(e->type){
	case SDL_TEXTINPUT:
		textInputAppend(e->text.text);
		break;
	case SDL_TEXTEDITING:
		//fprintf(stderr,"composition: %s %i %i\n",e->edit.text, e->edit.start, e->edit.length);
		break;
	case SDL_KEYUP:
		switch(e->key.keysym.sym){
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			textInputEnter();
			return true;
		}
		break;
	case SDL_KEYDOWN:
		switch(e->key.keysym.sym){
		case SDLK_BACKSPACE:
			textInputBackspace(0);
			return true;
		case SDLK_DELETE:
			textInputBackspace(1);
			return true;
		case SDLK_LEFT:
			textInputCheckMark(e);
			if(keyboardCmdKey(e)){
				textInputHome();
			}else if(textInputCursorPos > 0){
				--textInputCursorPos;
			}
			return true;
		case SDLK_RIGHT:
			textInputCheckMark(e);
			if(keyboardCmdKey(e)){
				textInputEnd();
			}else if(textInputCursorPos < textInputBufferLen){
				++textInputCursorPos;
			}
			return true;
		case SDLK_HOME:
			textInputCheckMark(e);
			textInputHome();
			return true;
		case SDLK_END:
			textInputCheckMark(e);
			textInputEnd();
			return true;
		case SDLK_INSERT:
			textInputPaste();
			return true;
		case SDLK_a:
			if(keyboardCmdKey(e)){
				textInputCursorPos = textInputBufferLen;
				textInputMark = 0;
				return true;
			}
			break;
		case SDLK_c:
			if(keyboardCmdKey(e)){
				textInputCopy();
				return true;
			}
			break;
		case SDLK_x:
			if(keyboardCmdKey(e)){
				textInputCut();
				return true;
			}
			break;
		case SDLK_v:
			if(keyboardCmdKey(e)){
				textInputPaste();
				return true;
			}
			break;
		}
	}
	return false;
}
