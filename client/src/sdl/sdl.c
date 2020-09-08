#include "sdl.h"
#include "../main.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/inventory.h"
#include "../misc/options.h"
#include "../sdl/sfx.h"

#include "../sdl/input_mouse.h"
#include "../sdl/input_keyboard.h"
#include "../sdl/input_gamepad.h"
#include "../sdl/input_touch.h"

#include <stdio.h>
#include <SDL.h>
#include <SDL_hints.h>
#ifdef EMSCRIPTEN
	#include <SDL2/SDL_mixer.h>
#else
	#include <SDL_mixer.h>
#endif

bool textInputEvent(const SDL_Event *e);

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;

int frameTimeLast = 0;
int frameCountSinceLast = 0;
float curFPS;

void fpsTick() {
	int curTicks = SDL_GetTicks();
	frameCountSinceLast++;

	if(curTicks > frameTimeLast+1000){
		curFPS = ((float)curTicks - (float)frameTimeLast) / (float)frameCountSinceLast;
		curFPS = 1000.f / curFPS;
		frameTimeLast += (curTicks - frameTimeLast);
		frameCountSinceLast = 0;
	}
}

void initSDL(){
	SDL_DisplayMode dm;

	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,"0");
	unsigned int initFlags = SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_AUDIO;
	if(SDL_Init( initFlags) < 0) {
		fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		exit(1);
	}

	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	#ifdef __EMSCRIPTEN__
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
	#else
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	#endif

	if( SDL_GetDesktopDisplayMode(0, &dm) == 0){
		screenWidth = dm.w;
		screenHeight = dm.h;
	}

	int cwflags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_CAPTURE | SDL_WINDOW_RESIZABLE;
	#ifdef __EMSCRIPTEN__
		cwflags |= SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_INPUT_GRABBED;
	#else
		if(optionFullscreen){
			cwflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}else{
			screenWidth = 800;
			screenHeight = 600;
		}
	#endif
	gWindow = SDL_CreateWindow( "Wolkenwelten", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, cwflags);
	if( gWindow == NULL ) {
		fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError() );
		exit(1);
	}

	gContext = SDL_GL_CreateContext( gWindow );
	if( gContext == NULL ){
		fprintf(stderr, "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
		exit(1);
	}
	#ifndef __HAIKU__
	if( SDL_GL_SetSwapInterval( 1 ) < 0 ){ //Use Vsync
		fprintf(stderr, "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
	}
	#endif

	mousex = screenWidth/2;
	mousey = screenHeight/2;
	SDL_ShowCursor(SDL_FALSE);
	SDL_DisableScreenSaver();

	if(Mix_OpenAudio(48000, MIX_DEFAULT_FORMAT, 2, 1024)==-1) {
		fprintf(stderr,"Mix_OpenAudio: %s\n", Mix_GetError());
	}
	Mix_AllocateChannels(128);
	sfxEnable = 1;

	initGL();
	gamepadInit();
}

void setFullscreen(bool fs){
	if(fs){
		SDL_SetWindowFullscreen(gWindow,SDL_WINDOW_FULLSCREEN_DESKTOP);
	}else{
		SDL_SetWindowFullscreen(gWindow,0);
		SDL_SetWindowSize(gWindow,800,600);
		SDL_SetWindowPosition(gWindow,SDL_WINDOWPOS_CENTERED ,SDL_WINDOWPOS_CENTERED);
	}
	optionFullscreen = fs;
}

void handleEvents(){
	SDL_Event e;

	while( SDL_PollEvent( &e ) != 0 ){
		switch(e.type){
		case SDL_QUIT:
			quit = true;
		break;

		case SDL_WINDOWEVENT:
			if((e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) || (e.window.event == SDL_WINDOWEVENT_RESIZED)){
				sdlResize(e.window.data1,e.window.data2);
			}
		break;
		}
		if(textInputEvent(&e)){continue;}
		mouseEventHandler(&e);
		keyboardEventHandler(&e);
		gamepadEventHandler(&e);
		touchEventHandler(&e);
	}
}

void sdlResize(int newW,int newH){
	if((newW == screenWidth) && (newH == screenHeight)){return;}
	screenWidth  = newW;
	screenHeight = newH;
	SDL_SetWindowSize(gWindow,screenWidth,screenHeight);
	initGL();
	resizeUI();
}

void closeSDL(){
	SDL_DestroyWindow( gWindow );
	closeGamepad();
	Mix_Quit();
	SDL_Quit();
}

void swapWindow(){
	SDL_GL_SwapWindow(gWindow);
}

void warpMouse(int mx,int my){
	SDL_WarpMouseInWindow(gWindow,mx,my);
}

void setRelativeMouseMode(bool ra){
	SDL_SetRelativeMouseMode(ra == true ? SDL_TRUE : SDL_FALSE);
}

unsigned int getMouseState(int *mx, int *my){
	return SDL_GetMouseState(mx,my);
}

unsigned int getTicks(){
	return SDL_GetTicks();
}

bool inputSneak(){
	return gamepadSneak()     || keyboardSneak()     || mouseSneak()     || touchSneak();
}
bool inputPrimary(){
	return gamepadPrimary()   || keyboardPrimary()   || mousePrimary()   || touchPrimary();
}
bool inputSecondary(){
	return gamepadSecondary() || keyboardSecondary() || mouseSecondary() || touchSecondary();
}
bool inputTertiary(){
	return gamepadTertiary()  || keyboardTertiary()  || mouseTertiary()  || touchTertiary();
}
