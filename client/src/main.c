/*
 * Wolkenwelten - Copyright (C) 2020- 2021 - Benjamin Vincent Schulenburg
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

#include "game/being.h"
#include "game/blockMining.h"
#include "game/blockType.h"
#include "game/character/character.h"
#include "game/entity.h"
#include "game/fire.h"
#include "game/fluid.h"
#include "game/projectile.h"
#include "game/rope.h"
#include "game/weather/weather.h"
#include "gfx/fluid.h"
#include "gfx/gfx.h"
#include "gfx/mesh.h"
#include "gfx/particle.h"
#include "gfx/shader.h"
#include "gfx/shadow.h"
#include "gfx/sky.h"
#include "gfx/texture.h"
#include "gui/gui.h"
#include "gui/menu.h"
#include "gui/menu/mainmenu.h"
#include "gui/overlay.h"
#include "nujel/nujel.h"
#include "misc/options.h"
#include "sdl/input/keyboard.h"
#include "sdl/sdl.h"
#include "sfx/environment.h"
#include "sfx/sfx.h"
#include "tmp/objs.h"
#include "voxel/bigchungus.h"
#include "voxel/chungus.h"

#include "../../common/src/asm/asm.h"
#include "../../common/src/game/time.h"
#include "../../common/src/misc/misc.h"
#include "../../common/src/misc/profiling.h"

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
bool singleplayer      = false;
u64  gameTicks = 0;

const char *gameModuleName = NULL;

void startGame(const char *moduleName){
	gtimeSetTime(1<<19);
	gameModuleName = moduleName;
	printf("Starting '%s'\n", moduleName);
	closeAllMenus();
	player = characterNew();
	gameRunning = true;
	widgetFocus(widgetGameScreen);
	lispCallFunc("on-join-fire", NULL);
}

void closeGame(){
	gameModuleName = NULL;
}

void playerInit(){
	if(player){characterFree(player);}
	player = characterNew();
}

void playerFree(){
	if(!player){return;}
	characterFree(player);
	player = NULL;
}

void signalQuit(int signo){
	(void)signo;
	if(quit){
		fprintf(stderr,"[CLI] Exiting immediatly due to multiple signals\n");
		exit(0);
	}
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
	if(player == NULL){return;}
	environmentSoundsUpdate();
	player->controls = vecZero();
	if(player->flags & CHAR_SPAWNING){ return; }
	lispInputTick();
	characterMove(player,player->controls);

	worldFreeFarChungi(player);
}

void worldDoTick(){
	PROFILE_START();
	characterUpdateAll();
	particleUpdate();
	ropeUpdateAll();
	projectileUpdateAll();
	gtimeUpdate();
	weatherUpdateAll();
	entityUpdateAll();
	lispEvents();
	fluidPhysicsTick();
	fireUpdateAll();

	gameTicks++;
	PROFILE_STOP();
}

void worldUpdate(){
	static int lastTick = 0;
	int curTick;

	if(lastTick == 0){lastTick = SDL_GetTicks();}
	curTick = SDL_GetTicks();
	resetOverlayColor();
	for(;lastTick < curTick;lastTick+=msPerTick){
		worldDoTick();
	}
	commitOverlayColor();
	sfxResetBeingBlocker();
}

static void UIStuff(){
	handleEvents();
	widgetUpdateAllEvents();
}

void mainloop(){
	UIStuff();
	if(gameRunning){
		playerUpdate();
		worldUpdate();
	}
	renderFrame();
	#ifdef __EMSCRIPTEN__
	if(quit && !gameRunning){emscripten_cancel_main_loop();}
	#endif
}


int main(int argc, char* argv[]){
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	asmDetermineSupport();
	initSignals();
	seedRNG(time(NULL));

	lispInit();
	initOptions(argc,argv);
	changeToDataDir();  // Change to data dir after parsing args so we can add an argument later to set the data dir
	loadOptions();
	initSDL();

	shaderInit();
	textureInit();
	sfxInit();
	beingListEntryInit();
	chunkInit();
	chungusInit();
	weatherInit();

	initGUI();
	initSky();
	shadowInit();
	blockTypeInit();
	blockMiningInit();
	initMeshobjs();
	particleInit();

	lispCallFunc("on-init-fire", NULL);
	textureBuildBlockIcons(0);
	ropeInit();

	cloudsInitGfx();
	rainInitGfx();
	snowInitGfx();
	if(gameModuleName){
		startGame(gameModuleName);
	}

	if(!gameRunning){openMainMenu();}
	#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop(mainloop, 0, true);
	#else
		while(!quit || gameRunning){ mainloop(); }
	#endif

	return 0;
}

void exitCleanly(){
	sfxFreeAll();
	meshFreeAll();
	textureFree();
	shaderFree();
	closeSDL();
	printf("[CLI] Exiting cleanly\n");
}
