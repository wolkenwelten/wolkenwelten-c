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

#include "mouse.h"

#include "../sdl.h"
#include "../../gui/gui.h"
#include "../../nujel/nujel.h"
#include "../../misc/options.h"
#include "../../game/character/character.h"
#include "../../main.h"
#include "../../../../common/nujel/lib/api.h"

void mouseEventHandler(const SDL_Event *e){
	static lSymbol *keyInput = NULL;
	if(keyInput == NULL){keyInput = lSymS("input-mouse-event");}

	switch(e->type){
	case SDL_MOUSEBUTTONDOWN:
		lispInputHandler(keyInput,e->button.button-1,1);
		break;
	case SDL_MOUSEBUTTONUP:
		lispInputHandler(keyInput,e->button.button-1,0);
		break;
	case SDL_MOUSEWHEEL:
		lispInputHandler(keyInput,5,e->wheel.y);
		break;
	case SDL_MOUSEMOTION:
		if(mouseHidden && !gameControlsInactive()){
			float mouseSpeed = (0.25f / player->zoomFactor) * optionMouseSensitivy;
			characterRotate(player,vecNew(e->motion.xrel*mouseSpeed,e->motion.yrel*mouseSpeed,0));
		}
		break;
	}
}
