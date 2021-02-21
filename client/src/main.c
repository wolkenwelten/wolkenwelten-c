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

#include "main.h"

#include "game/animal.h"
#include "game/being.h"
#include "game/blockMining.h"
#include "game/blockType.h"
#include "game/character.h"
#include "game/entity.h"
#include "game/fire.h"
#include "game/grenade.h"
#include "game/itemDrop.h"
#include "game/projectile.h"
#include "game/rain.h"
#include "game/recipe.h"
#include "game/rope.h"
#include "game/throwable.h"
#include "game/time.h"
#include "game/weather.h"
#include "gfx/gfx.h"
#include "gfx/mesh.h"
#include "gfx/particle.h"
#include "gfx/shader.h"
#include "gfx/shadow.h"
#include "gfx/sky.h"
#include "gfx/texture.h"
#include "gui/gui.h"
#include "gui/menu.h"
#include "gui/overlay.h"
#include "menu/inventory.h"
#include "menu/mainmenu.h"
#include "misc/lisp.h"
#include "misc/options.h"
#include "network/chat.h"
#include "network/client.h"
#include "sdl/input_gamepad.h"
#include "sdl/input_keyboard.h"
#include "sdl/input_touch.h"
#include "sdl/sdl.h"
#include "sdl/sfx.h"
#include "tmp/objs.h"
#include "voxel/bigchungus.h"
#include "voxel/chungus.h"

#include "../../common/src/game/itemType.h"
#include "../../common/src/misc/test.h"
#include "../../common/src/misc/misc.h"
#include "../../common/src/mods/mods.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#endif

bool isClient          = true;
bool quit              = false;
bool gameRunning       = false;
bool playerChunkActive = false;
bool singleplayer      = false;
bool chnkChngOverflow  = false;

void signalQuit(int signo){
	(void)signo;
	quit = true;
}

void initSignals(){
	#ifdef __MINGW32__
	signal(SIGTERM, signalQuit);
	signal(SIGABRT, signalQuit);
	#else
	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL, signalQuit);
	signal(SIGSTOP, signalQuit);
	signal(SIGHUP,  signalQuit);
	#endif
	signal(SIGINT,  signalQuit);
}

void playerUpdate(){
	vec nv = vecZero();
	chungus *chng = worldGetChungus((int)player->pos.x >> 8,(int)player->pos.y >> 8,(int)player->pos.z >> 8);
	if(chng != NULL){ playerChunkActive = chng->requested == 0; }
	if(!vecInWorld(player->pos)){ playerChunkActive = true; }
	if(player->flags & CHAR_SPAWNING){ return; }
	if(!playerChunkActive)           { return; }
	if(inputSneak()){
		player->flags |= CHAR_SNEAK;
	}else{
		player->flags &= ~CHAR_SNEAK;
	}
	if(inputBoost()){
		player->flags |= CHAR_BOOSTING;
	}else{
		player->flags &= ~CHAR_BOOSTING;
	}

	nv = doKeyboardupdate (nv);
	nv = doTouchupdate    (nv);
	nv = doGamepadupdate  (nv);
	characterMove  (player,nv);
	msgSendPlayerPos();
}

void worldUpdate(){
	static uint calls = 0;
	static  int lastTick = 0;
	int curTick;

	if(lastTick == 0){lastTick = SDL_GetTicks();}
	if(!playerChunkActive){lastTick = SDL_GetTicks();return;}
	curTick = SDL_GetTicks();
	resetOverlayColor();
	for(;lastTick < curTick;lastTick+=msPerTick){
		if(!isInventoryOpen()){
			if(inputPrimary()){
				characterPrimary(player);
			}else{
				characterStopMining(player);
			}
			if(inputSecondary()){characterSecondary(player);}
			if(inputTertiary()){characterTertiary(player);}
			if(inputThrow()){characterThrow(player);}
		}
		charactersUpdate();
		grenadeUpdateAll();
		animalUpdateAll();
		entityUpdateAll();
		itemDropUpdateAll();
		particleUpdate();
		ropeUpdateAll();
		projectileUpdateAll();
		gtimeUpdate();
		fireCheckPlayerBurn(calls);
		weatherUpdateAll();
		rainUpdateAll();
		throwableUpdateAll();
		throwableCheckPickup();
		lispEvents();

		calls++;
	}
	commitOverlayColor();
}

void mainloop(){
	handleEvents();
	if(gameRunning){
		clientTranceive();
		playerUpdate();
		worldUpdate();
		renderFrame();
		if(chnkChngOverflow){
			setRenderDistance(renderDistance*0.9f);
			chnkChngOverflow = false;
		}
		worldFreeFarChungi(player);
		clientTranceive();
		if(quit){clientGoodbye();}
	}else{
		doGamepadMenuUpdate();
		renderFrame();
	}
	#ifdef __EMSCRIPTEN__
	if(quit && !gameRunning){emscripten_cancel_main_loop();}
	#endif
}

void checkAutostart(){
	if(optionSavegame[0] != 0){
		startSingleplayer();
	}else if(serverName[0] != 0){
		startMultiplayer();
	}
}

int main( int argc, char* argv[] ){
	selfTest();
	clientGetName();
	clientGetServerExecutable();
	lispInit();
	initOptions(argc,argv);
	changeToDataDir();  // Change to data dir after parsing args so we can add an argument later to set the data dir
	loadOptions();
	initSDL();
	seedRNG(time(NULL));

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	initSignals();

	shaderInit();
	textureInit();
	sfxInit();
	beingListEntryInit();
	chunkInit();
	chungusInit();
	weatherInit();

	initMenu();
	initUI();
	initSky();
	shadowInit();
	blockTypeInit();
	blockMiningInit();
	initMeshobjs();
	particleInit();

	itemTypeInit();
	lispEval("(event-fire \"on-init\")");
	recipeInit();
	modsInit();
	textureBuildBlockIcons(0);
	ropeInit();

	if(!gameRunning){
		openMainMenu();
	}
	cloudsInitGfx();
	rainInitGfx();

	player = characterNew();
	initInventory();
	gtimeSetTime(1<<19);
	checkAutostart();
	#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop(mainloop, 0, true);
	#else
		while(!quit || gameRunning){ mainloop(); }
	#endif

	sfxFreeAll();
	meshFreeAll();
	textureFree();
	shaderFree();
	clientFree();
	closeSDL();
	closeSingleplayerServer();
	printf("[CLI] Exiting cleanly\n");
	return 0;
}
