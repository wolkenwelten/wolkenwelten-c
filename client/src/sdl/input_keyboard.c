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

#include "input_keyboard.h"

#include "sdl.h"
#include "../main.h"
#include "../game/character.h"
#include "../gfx/texture.h"
#include "../gfx/gfx.h"
#include "../gui/chat.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gui/menu.h"
#include "../gui/lispInput.h"
#include "../menu/inventory.h"
#include "../misc/lisp.h"
#include "../misc/options.h"
#include "../gui/widget.h"
#include "../misc/options.h"
#include "../network/chat.h"

void textInputEvent(const SDL_Event *e);
bool keysPressed[16];

bool keyboardSneak(){
	if(gameControlsInactive()){return false;}
	return keysPressed[8];
}
bool keyboardBoost(){
	if(gameControlsInactive()){return false;}
	return keysPressed[5];
}
bool keyboardPrimary(){
	if(gameControlsInactive()){return false;}
	return keysPressed[10];
}
bool keyboardSecondary(){
	if(gameControlsInactive()){return false;}
	return keysPressed[11];
}
bool keyboardTertiary(){
	if(gameControlsInactive()){return false;}
	return keysPressed[7];
}

bool keyboardThrow(){
	if(gameControlsInactive()){return false;}
	return keysPressed[6];
}

vec doKeyboardupdate(vec vel){
	if(gameControlsInactive()){return vel;}

	if(keysPressed[0]){ vel.z = -1.f; }
	if(keysPressed[1]){ vel.z =  1.f; }
	if(keysPressed[2]){ vel.x = -1.f; }
	if(keysPressed[3]){ vel.x =  1.f; }
	if(keysPressed[4]){ vel.y =  1.f; }

	vec rot = vecZero();
	if(keysPressed[12]){rot.x = -1;}
	if(keysPressed[13]){rot.y =  1;}
	if(keysPressed[14]){rot.y = -1;}
	if(keysPressed[15]){rot.x =  1;}
	float rotSpeed = 2.f / player->zoomFactor;
	characterRotate(player,vecMulS(rot,rotSpeed));

	/*
	if(keysPressed[6]){
		characterDropSingleItem(player,player->activeItem);
	}*/
	return vel;
}

int keyboardCmdKey(const SDL_Event *e){
	#ifdef __APPLE__
	return e->key.keysym.mod & KMOD_GUI;
	#elif __HAIKU__
	return e->key.keysym.mod & KMOD_ALT;
	#else
	return e->key.keysym.mod & KMOD_CTRL;
	#endif
}

void keyboardEventHandler(const SDL_Event *e){
	static lSymbol *keyInput = NULL;
	if(keyInput == NULL){keyInput = lSymS("input-keyboard-handler");}

	if(e->type == SDL_KEYUP){
		if(!textInputActive()){
			lispInputHandler(keyInput,e->key.keysym.scancode,0);
		}
		switch(e->key.keysym.scancode){
		case SDL_SCANCODE_UP:
		case SDL_SCANCODE_W:
			keysPressed[0] = 0;
			menuChangeFocus(0,1, e->key.keysym.sym == SDLK_w);
			break;
		case SDL_SCANCODE_DOWN:
		case SDL_SCANCODE_S:
			keysPressed[1] = 0;
			menuChangeFocus(0,-1, e->key.keysym.sym == SDLK_s);
			break;
		case SDL_SCANCODE_LEFT:
		case SDL_SCANCODE_A:
			keysPressed[2] = 0;
			menuChangeFocus(-1,0, e->key.keysym.sym == SDLK_a);
			break;
		case SDL_SCANCODE_RIGHT:
		case SDL_SCANCODE_D:
			keysPressed[3] = 0;
			menuChangeFocus(1,0, e->key.keysym.sym == SDLK_d);
			break;
		case SDL_SCANCODE_SPACE:
			keysPressed[4] = 0;
			break;
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_KP_ENTER:
			if(chatOpen()){
				menuKeyClick(0);
			}
			break;
		case SDL_SCANCODE_DELETE:
		case SDL_SCANCODE_BACKSPACE:
			menuKeyClick(1);
			break;
		case SDL_SCANCODE_LSHIFT:
			keysPressed[5] = 0;
			break;
		case SDL_SCANCODE_Q:
			keysPressed[6] = 0;
			break;
		case SDL_SCANCODE_R:
			keysPressed[7] = 0;
			break;
		case SDL_SCANCODE_V:
			keysPressed[8] = 0;
			break;
		case SDL_SCANCODE_G:
			keysPressed[9] = 0; // Unused
			break;
		case SDL_SCANCODE_Y:
			keysPressed[10] = 0;
			break;
		case SDL_SCANCODE_U:
			keysPressed[11] = 0;
			break;
		case SDL_SCANCODE_H:
			keysPressed[12] = 0;
			break;
		case SDL_SCANCODE_J:
			keysPressed[13] = 0;
			break;
		case SDL_SCANCODE_K:
			keysPressed[14] = 0;
			break;
		case SDL_SCANCODE_L:
			keysPressed[15] = 0;
			break;
		default:
			break;
		}
	}

	if(e->type == SDL_KEYDOWN){
		if(!textInputActive()){
			lispInputHandler(keyInput,e->key.keysym.scancode,1);
		}
		switch(e->key.keysym.scancode){
		case SDL_SCANCODE_UP:
		case SDL_SCANCODE_W:
			keysPressed[0] = 1;
			break;
		case SDL_SCANCODE_DOWN:
		case SDL_SCANCODE_S:
			keysPressed[1] = 1;
			break;
		case SDL_SCANCODE_LEFT:
		case SDL_SCANCODE_A:
			keysPressed[2] = 1;
			break;
		case SDL_SCANCODE_RIGHT:
		case SDL_SCANCODE_D:
			keysPressed[3] = 1;
			break;
		case SDL_SCANCODE_SPACE:
			keysPressed[4] = 1;
			break;
		case SDL_SCANCODE_LSHIFT:
			keysPressed[5] = 1;
			break;
		case SDL_SCANCODE_Q:
			keysPressed[6] = 1;
			if(keyboardCmdKey(e)){
				quit=true;
				keysPressed[6] = 0;
			}
			break;
		case SDL_SCANCODE_R:
			keysPressed[7] = 1;
			break;
		case SDL_SCANCODE_V:
			keysPressed[8] = 1;
			break;
		case SDL_SCANCODE_G:
			keysPressed[9] = 1;
			break;
		case SDL_SCANCODE_Y:
			keysPressed[10] = 1;
			break;
		case SDL_SCANCODE_U:
			keysPressed[11] = 1;
			break;
		case SDL_SCANCODE_H:
			keysPressed[12] = 1;
			break;
		case SDL_SCANCODE_J:
			keysPressed[13] = 1;
			break;
		case SDL_SCANCODE_K:
			keysPressed[14] = 1;
			break;
		case SDL_SCANCODE_L:
			keysPressed[15] = 1;
			break;
		case SDL_SCANCODE_F12:
		case SDL_SCANCODE_GRAVE:
			lispPanelOpen();
			break;
		case SDL_SCANCODE_ESCAPE:
			menuCancel();
			guiEscape();
			break;
		case SDL_SCANCODE_I:
		case SDL_SCANCODE_TAB:
			if(!textInputActive()){
				if(isInventoryOpen()){
					hideInventory();
				}else{
					showInventory();
				}
			}
			break;
		default:
			break;
		}
	}
}
