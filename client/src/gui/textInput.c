#define _DEFAULT_SOURCE
#include "textInput.h"
#include "../sdl/sdl.h"
#include "../gfx/textMesh.h"


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <SDL.h>

int  textInputLock = 0;
bool textInputActive = false;

int  textInputX = 0;
int  textInputY = 0;
int  textInputW = 0;
int  textInputH = 0;
char textInputBuffer[256];
int  textInputBufferLen = 0;
int  textInputCursorPos = 0;
textMesh *textInputMesh = NULL;

bool textInput(int x, int y, int w, int h, int lock){
	if(textInputLock != 0) {return false;}
	if(textInputActive)    {return false;}

	textInputActive    = true;
	textInputLock      = lock;
	textInputX         = x;
	textInputY         = y;
	textInputW         = w;
	textInputH         = h;
	textInputBufferLen = 0;
	textInputCursorPos = 0;
	memset(textInputBuffer,0,sizeof(textInputBuffer));
	if(textInputMesh == NULL){
		textInputMesh = textMeshNew();
	}

	SDL_StartTextInput();
	return true;
}

char *textInputGetBuffer(){
	return textInputBuffer;
}

void textInputClose(){
	SDL_StopTextInput();
	textInputActive = false;
}

void textInputDraw(){
	if(!textInputActive){return;}
	textMeshEmpty(textInputMesh);
	textInputMesh->sx   = textInputX;
	textInputMesh->sy   = textInputY;
	textInputMesh->size = 2;
	textMeshAddString(textInputMesh,textInputBuffer);
	if(getTicks() & 512){
		textMeshAddGlyph(textInputMesh, textInputX+(textInputCursorPos*16), textInputY, 2, 127);
	}
	textMeshDraw(textInputMesh);
}

void textInputBackspace(){
	if(!textInputActive){return;}
	if(textInputCursorPos == 0){return;}
	for(int i=textInputCursorPos-1;i<textInputBufferLen-1;i++){
		textInputBuffer[i] = textInputBuffer[i+1];
	}
	--textInputCursorPos;
	--textInputBufferLen;
	textInputBuffer[textInputBufferLen] = 0;
}

void textInputAppend(const char *s){
	int slen = strnlen(s,256);
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
}

bool textInputEvent(const SDL_Event *e){
	if(!textInputActive){return false;}

	switch(e->type){
	case SDL_TEXTINPUT:
		textInputAppend(e->text.text);
	break;

	case SDL_TEXTEDITING:
		fprintf(stderr,"composition: %s %i %i\n",e->edit.text, e->edit.start, e->edit.length);
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
		}
	break;
	}
	return false;
}
