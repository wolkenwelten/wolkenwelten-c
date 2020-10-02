#include "input_keyboard.h"

#include "sdl.h"
#include "../main.h"
#include "../game/character.h"
#include "../gfx/texture.h"
#include "../gui/gui.h"
#include "../gui/textInput.h"
#include "../gui/menu.h"
#include "../gui/inventory.h"
#include "../gui/widget.h"
#include "../misc/options.h"
#include "../network/chat.h"

void textInputEvent(const SDL_Event *e);

bool keysPressed[16];

bool keyboardSneak(){
	if(widgetFocused != NULL){return false;}
	return keysPressed[5];
}
bool keyboardPrimary(){
	if(widgetFocused != NULL){return false;}
	return keysPressed[10];
}
bool keyboardSecondary(){
	if(widgetFocused != NULL){return false;}
	return keysPressed[11];
}
bool keyboardTertiary(){
	if(widgetFocused != NULL){return false;}
	return keysPressed[7];
}

vec doKeyboardupdate(vec vel){
	if(widgetFocused != NULL){return vel;}

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
		switch(e->key.keysym.sym){
		case SDLK_UP:
		case SDLK_w:
			keysPressed[0] = 0;
			menuChangeFocus(0,1);
			break;
		case SDLK_DOWN:
		case SDLK_s:
			keysPressed[1] = 0;
			menuChangeFocus(0,-1);
			break;
		case SDLK_LEFT:
		case SDLK_a:
			keysPressed[2] = 0;
			menuChangeFocus(-1,0);
			break;
		case SDLK_RIGHT:
		case SDLK_d:
			keysPressed[3] = 0;
			menuChangeFocus(1,0);
			break;
		case SDLK_SPACE:
			keysPressed[4] = 0;
			menuKeyClick(0);
			break;
		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			openChat();
			menuKeyClick(0);
			break;
		case SDLK_DELETE:
		case SDLK_BACKSPACE:
			menuKeyClick(1);
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
		case SDLK_y:
			keysPressed[10] = 0;
			break;
		case SDLK_u:
			keysPressed[11] = 0;
			break;
		case SDLK_h:
			keysPressed[12] = 0;
			break;
		case SDLK_j:
			keysPressed[13] = 0;
			break;
		case SDLK_k:
			keysPressed[14] = 0;
			break;
		case SDLK_l:
			keysPressed[15] = 0;
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
	}

	if(e->type == SDL_KEYDOWN){
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
			if(keyboardCmdKey(e)){quit=true;}
			break;
		case SDLK_r:
			keysPressed[7] = 1;
			break;
		case SDLK_y:
			keysPressed[10] = 1;
			break;
		case SDLK_u:
			keysPressed[11] = 1;
			break;
		case SDLK_h:
			keysPressed[12] = 1;
			break;
		case SDLK_j:
			keysPressed[13] = 1;
			break;
		case SDLK_k:
			keysPressed[14] = 1;
			break;
		case SDLK_l:
			keysPressed[15] = 1;
			break;
		case SDLK_ESCAPE:
			menuCancel();
			guiCancel();
			break;
		}
	}

	if((widgetFocused != NULL) || (!gameRunning)){return;}
	if(e->type == SDL_KEYDOWN){
		switch(e->key.keysym.sym){
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
	}
}
