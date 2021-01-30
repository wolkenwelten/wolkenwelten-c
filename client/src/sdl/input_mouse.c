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

#include "input_mouse.h"

#include "../sdl/sdl.h"
#include "../gui/gui.h"
#include "../menu/inventory.h"
#include "../game/character.h"
#include "../main.h"

bool mouseButtonsPressed[5];

bool mouseSneak(){
	return false;
}
bool mouseBoost(){
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
			if(mouseHidden && !gameControlsInactive()){
				float mouseSpeed = 0.25f / (1.f + (player->aimFade * player->zoomFactor));
				characterRotate(player,vecNew(e->motion.xrel*mouseSpeed,e->motion.yrel*mouseSpeed,0));
			}
		break;

		case SDL_MOUSEWHEEL:
			if(mouseHidden && !gameControlsInactive()){
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
