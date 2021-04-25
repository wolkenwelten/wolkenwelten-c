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

#include "input_gamepad.h"
#include "../sdl/sdl.h"
#include "../game/character.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/widget.h"
#include "../menu/inventory.h"
#include "../main.h"
#include "../../../common/src/common.h"

#include <math.h>

bool sdlGamepadInit         = false;
bool sdlHapticInit          = false;
bool sdlGamepadEverPressed  = false;

SDL_GameController *gamepad = NULL;
SDL_Haptic *haptic          = NULL;

int gamepadI                = -1;
unsigned int hapticStart    =  0;

bool  gamepadActive         = false;
float gamepadLeftAxisX      = 0.f;
float gamepadLeftAxisY      = 0.f;
float gamepadRightAxisX     = 0.f;
float gamepadRightAxisY     = 0.f;
float gamepadLeftTrigger    = 0.f;
float gamepadRightTrigger   = 0.f;
bool  gamepadButtons[16];

bool gamepadSneak(){
	return gamepadButtons[7];
}
bool gamepadBoost(){
	return gamepadButtons[9];
}
bool gamepadPrimary(){
	return (gamepadRightTrigger > 0.5f);
}
bool gamepadSecondary(){
	return (gamepadLeftTrigger > 0.5f);
}
bool gamepadTertiary(){
	return gamepadButtons[1];
}
bool gamepadThrow(){
	return gamepadButtons[3];
}

void gamepadInit(){
	if(SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == 0){
		sdlGamepadInit = true;
		checkForGamepad();
	}
	if(SDL_Init(SDL_INIT_HAPTIC) == 0){
		sdlHapticInit = true;
		checkForHaptic();
	}
}

void closeGamepad(){
	if ((gamepad != NULL) && SDL_GameControllerGetAttached(gamepad)) {
		SDL_GameControllerClose(gamepad);
	}
	if(haptic != NULL){
		SDL_HapticClose(haptic);
	}
}

void checkForHaptic(){
	if(!sdlHapticInit){return;}
	if((haptic == NULL) && (gamepad != NULL)){
		SDL_Joystick *joy = SDL_GameControllerGetJoystick(gamepad);
		haptic = SDL_HapticOpenFromJoystick(joy);
		SDL_HapticRumbleInit(haptic);
	}
}

void vibrate(float force){
	if(!sdlGamepadEverPressed){return;}
	if(!sdlHapticInit || (haptic == NULL)){return;}
	SDL_HapticRumblePlay(haptic,force,80);
}

void checkForGamepad(){
	if(!sdlGamepadInit){return;}
	for(int i=0;i<SDL_NumJoysticks();i++){
		if(!SDL_IsGameController(i)){ continue; }
		gamepad = SDL_GameControllerOpen(i);
		if(gamepad){
			gamepadActive = true;
			gamepadI = i;
			SDL_GameControllerEventState(SDL_ENABLE);
			break;
		}
	}
	checkForHaptic();
}

void controllerDeviceEvent(const SDL_Event *e){
	if(!sdlGamepadInit){return;}
	switch(e->type){
		case SDL_CONTROLLERDEVICEADDED:
			if(gamepadActive){return;}
			checkForGamepad();
		break;

		case SDL_CONTROLLERDEVICEREMOVED:
			if(!gamepadActive){return;}
			if(e->cdevice.which == gamepadI){
				gamepadActive = false;
				if (SDL_GameControllerGetAttached(gamepad)) {
					SDL_GameControllerClose(gamepad);
				}
				gamepad = NULL;
				gamepadI = -1;
				haptic = NULL;
			}
		break;
	}
}

void doGamepadMenuUpdate(){
	if(gamepadButtons[0]){
		gamepadButtons[0] = false;
		menuKeyClick(0);
	}
	if(gamepadButtons[1]){
		gamepadButtons[1] = false;
		menuKeyClick(1);
	}
	if(gamepadButtons[4]){
		gamepadButtons[4] = false;
		menuCancel();
		guiCancel();
	}
	if(gamepadButtons[11]){
		gamepadButtons[11] = false;
		menuChangeFocus(0, 1, 0);
	}
	if(gamepadButtons[12]){
		gamepadButtons[12] = false;
		menuChangeFocus(0,-1, 0);
	}
	if(gamepadButtons[13]){
		gamepadButtons[13] = false;
		menuChangeFocus( 1,0, 0);
	}
	if(gamepadButtons[14]){
		gamepadButtons[14] = false;
		menuChangeFocus(-1,0, 0);
	}
}

vec doGamepadupdate(vec vel){
	static unsigned int lastDown[16] = {0};
	if(!gamepadActive){return vel;}
	if(isInventoryOpen() || ((widgetFocused != NULL) && (widgetFocused->type != wGameScreen))){
		doGamepadMenuUpdate();
	}
	if( fabsf(gamepadLeftAxisX) > 0.2f){ vel.x = gamepadLeftAxisX; }
	if( fabsf(gamepadLeftAxisY) > 0.2f){ vel.z = gamepadLeftAxisY; }

	if( (fabsf(gamepadRightAxisX) > 0.2f) || (fabsf(gamepadRightAxisY) > 0.2f)){
		int mx,my;
		mx = gamepadRightAxisX*4.f;
		my = gamepadRightAxisY*4.f;
		if(fabsf(gamepadRightAxisX) < 0.2f){ mx = 0;}
		if(fabsf(gamepadRightAxisY) < 0.2f){ my = 0;}
		if(!isInventoryOpen()){
			characterRotate(player,vecNew(mx,my,0));
		}
	}
	unsigned int curticks = getTicks();

	if(!gamepadButtons[0]){
		lastDown[0] = 0;
	}else if(gamepadButtons[0] && (curticks > (lastDown[0] + 600))){
		if(isInventoryOpen()){
			if(lastDown[0] == 0){
				lastDown[0] = curticks;
			}else{
				lastDown[0] += 50;
			}
		}else{
			vel.y = 1.f;
		}
	}

	if(gamepadButtons[2]){
		gamepadButtons[2] = false;
	}
	if(gamepadButtons[3]){
		gamepadButtons[3] = false;
	}
	if(gamepadButtons[6]){
		gamepadButtons[6] = false;
		if(isInventoryOpen()){
			hideInventory();
		}else{
			showInventory();
		}
	}
	if(gamepadButtons[4]){
		gamepadButtons[4] = false;
		menuCancel();
		guiEscape();
	}
	if(gamepadButtons[5]){
		gamepadButtons[5] = false;
		menuCancel();
		guiEscape();
	}

	if(gamepadButtons[10]){
		gamepadButtons[10] = false;
		characterFireHook(player);
	}

	if(gamepadButtons[11]){
		gamepadButtons[11] = false;
	}
	if(gamepadButtons[12]){
		gamepadButtons[12] = false;
	}
	if(gamepadButtons[13]){
		gamepadButtons[13] = false;

		if(!isInventoryOpen()){
			unsigned int nai = player->activeItem-1;
			if(nai > 9){nai=9;}
			characterSetActiveItem(player,nai);
		}
	}
	if(gamepadButtons[14]){
		gamepadButtons[14] = false;

		if(!isInventoryOpen()){
			unsigned int nai = player->activeItem+1;
			if(nai > 9){nai=0;}
			characterSetActiveItem(player,nai);
		}
	}
	return vel;
}

void gamepadEventHandler(const SDL_Event *e){
	switch(e->type){
	case SDL_CONTROLLERDEVICEADDED:
	case SDL_CONTROLLERDEVICEREMOVED:
	case SDL_CONTROLLERDEVICEREMAPPED:
		controllerDeviceEvent(e);
	break;

	case SDL_CONTROLLERAXISMOTION:
		switch(e->caxis.axis){
		case 0:
			gamepadLeftAxisX = ((float)e->caxis.value/32768.f);
			break;
		case 1:
			gamepadLeftAxisY = ((float)e->caxis.value/32768.f);
			break;
		case 2:
			gamepadRightAxisX = ((float)e->caxis.value/32768.f);
			break;
		case 3:
			gamepadRightAxisY = ((float)e->caxis.value/32768.f);
			break;
		case 4:
			gamepadLeftTrigger = ((float)e->caxis.value/32768.f);
			break;
		case 5:
			gamepadRightTrigger = ((float)e->caxis.value/32768.f);
			break;
		}
		break;
	case SDL_CONTROLLERBUTTONDOWN:
		gamepadButtons[e->cbutton.button] = true;
		sdlGamepadEverPressed = true;
		break;
	case SDL_CONTROLLERBUTTONUP:
		gamepadButtons[e->cbutton.button] = false;
		break;
	}
}
