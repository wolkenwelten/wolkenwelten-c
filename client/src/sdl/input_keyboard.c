#include "input_keyboard.h"

#include "sdl.h"
#include "../main.h"
#include "../game/character.h"
#include "../gfx/texture.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gui/menu.h"
#include "../menu/inventory.h"
#include "../gui/widget.h"
#include "../misc/options.h"
#include "../network/chat.h"

void textInputEvent(const SDL_Event *e);
bool keysPressed[16];

bool keyboardSneak(){
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
	characterRotate(player,vecMulS(rot,3));

	if(keysPressed[6]){
		characterDropItem(player,player->activeItem);
	}
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
	if(e->type == SDL_KEYUP){
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
			menuKeyClick(0);
			break;
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_KP_ENTER:
			openChat();
			menuKeyClick(0);
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
		#ifndef __EMSCRIPTEN__
		case SDL_SCANCODE_F5:
			textureReload();
			break;

		case SDL_SCANCODE_F11:
			setFullscreen(!optionFullscreen);
			break;
		#endif

		default:
			break;
		}
	}

	if(e->type == SDL_KEYDOWN){
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
		case SDL_SCANCODE_ESCAPE:
			menuCancel();
			guiCancel();
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
		case SDL_SCANCODE_F2:
			showInventory();
			break;
		case SDL_SCANCODE_F3:
			showCrafting();
			break;
		default:
			break;
		}
	}

	if(gameControlsInactive()){return;}
	if(e->type == SDL_KEYDOWN){
		switch(e->key.keysym.scancode){
		case SDL_SCANCODE_E:
			characterFireHook(player);
			break;
		case SDL_SCANCODE_1:
			characterSetActiveItem(player,0);
			break;
		case SDL_SCANCODE_2:
			characterSetActiveItem(player,1);
			break;
		case SDL_SCANCODE_3:
			characterSetActiveItem(player,2);
			break;
		case SDL_SCANCODE_4:
			characterSetActiveItem(player,3);
			break;
		case SDL_SCANCODE_5:
			characterSetActiveItem(player,4);
			break;
		case SDL_SCANCODE_6:
			characterSetActiveItem(player,5);
			break;
		case SDL_SCANCODE_7:
			characterSetActiveItem(player,6);
			break;
		case SDL_SCANCODE_8:
			characterSetActiveItem(player,7);
			break;
		case SDL_SCANCODE_9:
			characterSetActiveItem(player,8);
			break;
		case SDL_SCANCODE_0:
			characterSetActiveItem(player,9);
			break;
		case SDL_SCANCODE_M:
			optionDebugInfo = 1 - optionDebugInfo;
			break;
		case SDL_SCANCODE_N:
			player->flags ^= CHAR_NOCLIP;
			break;
		case SDL_SCANCODE_V:
			player->flags ^= CHAR_GLIDE;
			break;
		default:
			break;
		}
	}
}
