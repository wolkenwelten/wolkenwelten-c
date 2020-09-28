#include "input_mouse.h"

#include "../gui/gui.h"
#include "../gui/inventory.h"
#include "../game/character.h"
#include "../main.h"

bool mouseButtonsPressed[5];

bool mouseSneak(){
	return false;
}
bool mousePrimary(){
	if(!mouseHidden){return 0;}
	return mouseButtonsPressed[0];
}
bool mouseSecondary(){
	if(!mouseHidden){return 0;}
	return mouseButtonsPressed[2];
}
bool mouseTertiary(){
	return false;
}

void mouseEventHandler(const SDL_Event *e){
	switch(e->type){
		case SDL_MOUSEBUTTONDOWN:
			switch(e->button.button){
				case SDL_BUTTON_LEFT:
					mouseButtonsPressed[0] = 1;
				break;
				case SDL_BUTTON_MIDDLE:
					mouseButtonsPressed[1] = 1;
				break;
				case SDL_BUTTON_RIGHT:
					mouseButtonsPressed[2] = 1;
				break;
				case SDL_BUTTON_X1:
					mouseButtonsPressed[3] = 1;
				break;
				case SDL_BUTTON_X2:
					mouseButtonsPressed[4] = 1;
				break;
			}
		break;

		case SDL_MOUSEBUTTONUP:
			switch(e->button.button){
				case SDL_BUTTON_LEFT:
					mouseButtonsPressed[0] = 0;
				break;
				case SDL_BUTTON_MIDDLE:
					mouseButtonsPressed[1] = 0;
				break;
				case SDL_BUTTON_RIGHT:
					mouseButtonsPressed[2] = 0;
				break;
				case SDL_BUTTON_X1:
					mouseButtonsPressed[3] = 0;
				break;
				case SDL_BUTTON_X2:
					mouseButtonsPressed[4] = 0;
				break;
			}
		break;

		case SDL_MOUSEMOTION:
			if(mouseHidden && gameRunning){
				characterRotate(player,vecNew(e->motion.xrel/4.f,e->motion.yrel/4.f,0));
			}
		break;

		case SDL_MOUSEWHEEL:
			if(mouseHidden && gameRunning){
				uint nai = player->activeItem;
				if(e->wheel.y > 0){
					if(--nai > 9){nai = 9;}
				}else if(e->wheel.y < 0){
					if(++nai > 9){nai = 0;}
				}
				characterSetActiveItem(player,nai);
			}
		break;
	}
}
