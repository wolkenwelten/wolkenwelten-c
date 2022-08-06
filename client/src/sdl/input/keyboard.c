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
#include "keyboard.h"

#include "../sdl.h"
#include "../../main.h"
#include "../../game/character/character.h"
#include "../../gfx/texture.h"
#include "../../gfx/gfx.h"
#include "../../gui/gui.h"
#include "../../gui/textInput.h"
#include "../../gui/menu.h"
#include "../../misc/options.h"
#include "../../nujel/nujel.h"
#include "../../gui/widget.h"
#include "../../misc/options.h"
#include "../../../../common/nujel/lib/api.h"

bool textInputEvent(const SDL_Event *e);

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
	if(keyInput == NULL){keyInput = lSymS("input-keyboard-event");}

	if(e->type == SDL_KEYUP){
		lispInputHandler(keyInput,e->key.keysym.scancode,0);
	}
	if(e->type == SDL_KEYDOWN){
		if(!textInputActive()){
			lispInputHandler(keyInput,e->key.keysym.scancode,1);
		}
		switch(e->key.keysym.scancode){
		case SDL_SCANCODE_Q:
			if(keyboardCmdKey(e)){
				exitCleanly();
			}
			break;
		case SDL_SCANCODE_F12:
		case SDL_SCANCODE_GRAVE:
			//lispPanelOpen();
			break;
		case SDL_SCANCODE_ESCAPE:
			if(menuCancel()){break;}
			guiEscape();
			break;
		case SDL_SCANCODE_DELETE:
		case SDL_SCANCODE_BACKSPACE:
			if(!textInputActive()){menuKeyClick(1);}
			break;
		case SDL_SCANCODE_RETURN:
		case SDL_SCANCODE_KP_ENTER:
			if(!textInputActive()){menuKeyClick(0);}
			break;
		case SDL_SCANCODE_UP:
		case SDL_SCANCODE_W:
			menuChangeFocus(0,1, e->key.keysym.sym == SDLK_w);
			break;
		case SDL_SCANCODE_DOWN:
		case SDL_SCANCODE_S:
			menuChangeFocus(0,-1, e->key.keysym.sym == SDLK_s);
			break;
		case SDL_SCANCODE_LEFT:
		case SDL_SCANCODE_A:
			menuChangeFocus(-1,0, e->key.keysym.sym == SDLK_a);
			break;
		case SDL_SCANCODE_RIGHT:
		case SDL_SCANCODE_D:
			menuChangeFocus(1,0, e->key.keysym.sym == SDLK_d);
			break;
		default:
			break;
		}
	}
}
