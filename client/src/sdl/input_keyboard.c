#include "input_keyboard.h"

#include "sdl.h"
#include "../gfx/texture.h"
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
bool keyboardPrimary(){
	return false;
}
bool keyboardSecondary(){
	return false;
}
bool keyboardTertiary(){
	return keysPressed[7];
}

vec doKeyboardupdate(vec vel){
	if(keysPressed[0]){ vel.z = -1.f; }
	if(keysPressed[1]){ vel.z =  1.f; }
	if(keysPressed[2]){ vel.x = -1.f; }
	if(keysPressed[3]){ vel.x =  1.f; }
	if(keysPressed[4]){ vel.y =  1.f; }

	if(keysPressed[6]){
		characterDropItem(player,player->activeItem);
	}
	return vel;
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
		}else if(gameRunning){
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
				case SDLK_r:
					keysPressed[7] = 0;
				break;

				#ifndef __EMSCRIPTEN__
				case SDLK_F5:
					textureReload();
				break;

				case SDLK_F11:
					setFullscreen(!optionFullscreen);
				break;
				#endif

				default:
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
				case SDLK_r:
					keysPressed[7] = 1;
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
					characterSetActiveItem(player,0);
				break;
				case SDLK_2:
					characterSetActiveItem(player,1);
				break;
				case SDLK_3:
					characterSetActiveItem(player,2);
				break;
				case SDLK_4:
					characterSetActiveItem(player,3);
				break;
				case SDLK_5:
					characterSetActiveItem(player,4);
				break;
				case SDLK_6:
					characterSetActiveItem(player,5);
				break;
				case SDLK_7:
					characterSetActiveItem(player,6);
				break;
				case SDLK_8:
					characterSetActiveItem(player,7);
				break;
				case SDLK_9:
					characterSetActiveItem(player,8);
				break;
				case SDLK_0:
					characterSetActiveItem(player,9);
				break;
				case SDLK_m:
					optionDebugInfo = 1 - optionDebugInfo;
				break;
				case SDLK_n:
					player->flags ^= CHAR_NOCLIP;
				break;
				case SDLK_v:
					player->flags ^= CHAR_GLIDE;
				break;
			}
		break;
	}
}
