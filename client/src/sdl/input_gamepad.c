#include "input_gamepad.h"
#include "sdl.h"
#include "../game/character.h"
#include "../gui/menu.h"
#include "../gui/inventory.h"
#include "../main.h"

#include <math.h>

bool sdlGamepadInit = false;
bool sdlHapticInit  = false;

SDL_GameController *gamepad = NULL;
SDL_Haptic *haptic = NULL;

int gamepadI = -1;
unsigned int hapticStart = 0;

bool  gamepadActive       = false;
float gamepadLeftAxisX    = 0.f;
float gamepadLeftAxisY    = 0.f;
float gamepadRightAxisX   = 0.f;
float gamepadRightAxisY   = 0.f;
float gamepadLeftTrigger  = 0.f;
float gamepadRightTrigger = 0.f;
bool  gamepadButtons[16];

bool gamepadSneak(){
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
		updateMenuGamepad(1);
	}
	if(gamepadButtons[11]){
		gamepadButtons[11] = false;
		changeMenuSelection(-1);
	}
	if(gamepadButtons[12]){
		gamepadButtons[12] = false;
		changeMenuSelection(1);
	}
}

void doGamepadupdate(float *vx,float *vy,float *vz){
	static unsigned int lastDown[16] = {0};
	if(!gamepadActive){return;}
	if( fabsf(gamepadLeftAxisX) > 0.2f){ *vx = gamepadLeftAxisX; }
	if( fabsf(gamepadLeftAxisY) > 0.2f){ *vz = gamepadLeftAxisY; }

	if( (fabsf(gamepadRightAxisX) > 0.2f) || (fabsf(gamepadRightAxisY) > 0.2f)){
		int mx,my;
		mx = gamepadRightAxisX*4.f;
		my = gamepadRightAxisY*4.f;
		if(fabsf(gamepadRightAxisX) < 0.2f){ mx = 0;}
		if(fabsf(gamepadRightAxisY) < 0.2f){ my = 0;}
		if(!isInventoryOpen()){
			characterRotate(player,(float)mx,(float)my,0.f);
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
			updateInventoryGamepad(1);
		}else{
			*vy = 1.f;
		}
	}

	if(gamepadButtons[2]){
		gamepadButtons[2] = false;
		if(isInventoryOpen()){
			updateInventoryGamepad(2);
		}
	}
	if(gamepadButtons[3]){
		gamepadButtons[3] = false;
		if(isInventoryOpen()){
			updateInventoryGamepad(3);
		}else{
			characterDropItem(player,player->activeItem);
		}
	}
	if(gamepadButtons[6]){
		gamepadButtons[6] = false;
		if(isInventoryOpen()){
			hideInventory();
		}else{
			showInventory();
		}
	}
	if(gamepadButtons[9]){
		if(player->flags & CHAR_FALLING){
			gamepadButtons[9] = false;
			player->flags ^= CHAR_GLIDE;
		}
	}
	if(gamepadButtons[10]){
		gamepadButtons[10] = false;
		characterFireHook(player);
	}

	if(gamepadButtons[11]){
		gamepadButtons[11] = false;
		changeMenuSelection(1);
		if(isInventoryOpen()){
			changeInventorySelection(0,1);
		}
	}
	if(gamepadButtons[12]){
		gamepadButtons[12] = false;
		changeMenuSelection(-1);
		if(isInventoryOpen()){
			changeInventorySelection(0,-1);
		}
	}
	if(gamepadButtons[13]){
		gamepadButtons[13] = false;
		if(isInventoryOpen()){
			changeInventorySelection(-1,0);
		}else{
			if(--player->activeItem > 9){player->activeItem = 9;}
		}
	}
	if(gamepadButtons[14]){
		gamepadButtons[14] = false;
		if(isInventoryOpen()){
			changeInventorySelection(1,0);
		}else{
			if(++player->activeItem > 9){player->activeItem = 0;}
		}
	}
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
		break;

		case SDL_CONTROLLERBUTTONUP:
			gamepadButtons[e->cbutton.button] = false;
		break;
	}
}
