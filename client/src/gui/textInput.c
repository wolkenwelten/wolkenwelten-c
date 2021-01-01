#include "textInput.h"
#include "../sdl/input_keyboard.h"
#include "../sdl/sdl.h"
#include "../gfx/textMesh.h"
#include "../gui/widget.h"

#include <stdio.h>
#include <string.h>
#include <SDL.h>

int  textInputBufferLen = 0;
int  textInputCursorPos = 0;
bool textInputStarted = false;

void textInputFocus(widget *wid){
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

void textInputBackspace(){
	if(!textInputActive()){return;}
	if(textInputCursorPos == 0){return;}
	char *textInputBuffer = widgetFocused->vals;
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

void textInputPaste(){
	if(!textInputActive()){return;}
	if(!SDL_HasClipboardText()){return;}
	char *text = SDL_GetClipboardText();
	if(text == NULL){return;}
	textInputAppend(text);
	SDL_free(text);
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
			textInputBackspace();
			break;
		case SDLK_DELETE:
			if(textInputCursorPos < textInputBufferLen){
				++textInputCursorPos;
				textInputBackspace();
			}
			break;
		case SDLK_LEFT:
			if(textInputCursorPos > 0){
				--textInputCursorPos;
			}
			break;
		case SDLK_RIGHT:
			if(textInputCursorPos < textInputBufferLen){
				++textInputCursorPos;
			}
			break;
		case SDLK_INSERT:
			textInputPaste();
			return true;
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
