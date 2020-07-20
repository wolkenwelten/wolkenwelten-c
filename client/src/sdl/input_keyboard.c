#include "input_keyboard.h"

#include "../gui/textInput.h"
#include "../network/chat.h"
#include "../gui/inventory.h"
#include "../game/character.h"
#include "../main.h"
#include "../misc/options.h"

void textInputEvent(const SDL_Event *e);

bool keysPressed[16];

bool keyboardSneak(){
	return keysPressed[5];
}
bool keyboardMine(){
	return false;
}
bool keyboardActivate(){
	return false;
}

void doKeyboardupdate(float *vx,float *vy,float *vz){
	if(keysPressed[0]){ *vz = -1.f; }
	if(keysPressed[1]){ *vz =  1.f; }
	if(keysPressed[2]){ *vx = -1.f; }
	if(keysPressed[3]){ *vx =  1.f; }
	if(keysPressed[4]){ *vy =  1.f; }

	if(keysPressed[6]){
		characterDropItem(player,player->activeItem);
	}
}

void keyboardEventHandler(const SDL_Event *e){
	if((e->type == SDL_KEYDOWN) && (e->key.keysym.sym == SDLK_ESCAPE)){
		if(isInventoryOpen()){
			hideInventory();
		}else{
			quit = true;
		}
	}

	if((e->type == SDL_KEYUP) && ((e->key.keysym.sym == SDLK_RETURN) || (e->key.keysym.sym == SDLK_KP_ENTER))){
		if(textInputActive || (textInputLock != 0)){
			textInputClose();
		}else{
			chatStartInput();
		}
	}

	if(textInputActive){return;}

	switch(e->type){
		case SDL_KEYUP:
			switch(e->key.keysym.sym){
				case SDLK_UP:
				case SDLK_w:
					keysPressed[0] = 0;
				break;
				case SDLK_DOWN:
				case SDLK_s:
					keysPressed[1] = 0;
				break;
				case SDLK_LEFT:
				case SDLK_a:
					keysPressed[2] = 0;
				break;
				case SDLK_RIGHT:
				case SDLK_d:
					keysPressed[3] = 0;
				break;
				case SDLK_SPACE:
					keysPressed[4] = 0;
				break;
				case SDLK_LSHIFT:
					keysPressed[5] = 0;
				break;
				case SDLK_q:
					keysPressed[6] = 0;
				break;

				default:
				break;

				case SDLK_KP_ENTER:
				case SDLK_RETURN:
					chatStartInput();
				break;
			}
		break;

		case SDL_KEYDOWN:
			switch(e->key.keysym.sym){
				case SDLK_UP:
				case SDLK_w:
					keysPressed[0] = 1;
				break;
				case SDLK_DOWN:
				case SDLK_s:
					keysPressed[1] = 1;
				break;
				case SDLK_LEFT:
				case SDLK_a:
					keysPressed[2] = 1;
				break;
				case SDLK_RIGHT:
				case SDLK_d:
					keysPressed[3] = 1;
				break;
				case SDLK_SPACE:
					keysPressed[4] = 1;
				break;
				case SDLK_LSHIFT:
					keysPressed[5] = 1;
				break;
				case SDLK_q:
					keysPressed[6] = 1;
				break;
				case SDLK_e:
					characterFireHook(player);
				break;
				case SDLK_i:
				case SDLK_TAB:
					if(isInventoryOpen()){
						hideInventory();
					}else{
						showInventory();
					}
				break;

				case SDLK_1:
					player->activeItem = 0;
				break;
				case SDLK_2:
					player->activeItem = 1;
				break;
				case SDLK_3:
					player->activeItem = 2;
				break;
				case SDLK_4:
					player->activeItem = 3;
				break;
				case SDLK_5:
					player->activeItem = 4;
				break;
				case SDLK_6:
					player->activeItem = 5;
				break;
				case SDLK_7:
					player->activeItem = 6;
				break;
				case SDLK_8:
					player->activeItem = 7;
				break;
				case SDLK_9:
					player->activeItem = 8;
				break;
				case SDLK_0:
					player->activeItem = 9;
				break;
				case SDLK_m:
					optionDebugInfo = 1 - optionDebugInfo;
				break;
				case SDLK_n:
					player->noClip = 1 - player->noClip;
				break;
			}
		break;
	}
}
