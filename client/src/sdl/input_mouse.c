#include "input_mouse.h"

#include "../gui/inventory.h"
#include "../game/character.h"
#include "../main.h"

bool mouseButtonsPressed[5];

bool mouseSneak(){
	return false;
}
bool mouseMine(){
	return mouseButtonsPressed[0];
}
bool mouseActivate(){
	return mouseButtonsPressed[2];
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
			if(!isInventoryOpen() && gameRunning){
				characterRotate(player,((float)e->motion.xrel)/4.f,((float)e->motion.yrel)/4.f,0.f);
			}
		break;

		case SDL_MOUSEWHEEL:
			if(!isInventoryOpen() && gameRunning){
				if(e->wheel.y > 0){
					if(--player->activeItem > 9){player->activeItem = 9;}
				}else if(e->wheel.y < 0){
					if(++player->activeItem > 9){player->activeItem = 0;}
				}
			}
		break;
	}
}
