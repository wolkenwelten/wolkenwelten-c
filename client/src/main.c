#include "main.h"

#include "game/animal.h"
#include "game/blockMining.h"
#include "game/blockType.h"
#include "game/character.h"
#include "game/entity.h"
#include "game/fire.h"
#include "game/grenade.h"
#include "game/itemDrop.h"
#include "game/projectile.h"
#include "game/recipe.h"
#include "game/rope.h"
#include "game/time.h"
#include "gfx/clouds.h"
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
#include "misc/tests.h"
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
#include "../../common/src/misc/misc.h"
#include "../../common/src/mods/mods.h"

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
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
	nv = doAutomatedupdate(nv);
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
	for(;lastTick < curTick;lastTick+=MS_PER_TICK){
		if(!isInventoryOpen()){
			if(inputPrimary()){
				characterPrimary(player);
			}else{
				characterStopMining(player);
			}
			if(inputSecondary()){
				characterSecondary(player);
			}
			if(inputTertiary()){
				characterTertiary(player);
			}
		}
		resetOverlayColor();
		if(gameRunning && gameControlsInactive()){setOverlayColor(0x90182028,300);}
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

		commitOverlayColor();
		calls++;
	}
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
	}else{
		doGamepadMenuUpdate();
		renderFrame();
	}
}

void checkAutostart(){
	if(optionSavegame[0] != 0){
		startSingleplayer();
	}else if(serverName[0] != 0){
		startMultiplayer();
	}
}

int main( int argc, char* argv[] ){
	clientGetName();
	initOptions(argc,argv);
	initSDL();
	seedRNG(time(NULL));

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	initSignals();

	shaderInit();
	textureInit();
	sfxInit();
	chunkInit();
	chungusInit();

	initMenu();
	lispInit();
	initUI();
	initSky();
	shadowInit();
	cloudsInit();
	blockTypeInit();
	blockMiningInit();
	initMeshobjs();
	particleInit();
	recipeInit();

	modsInit();
	textureBuildBlockIcons(0);
	ropeInit();

	if(!gameRunning){
		openMainMenu();
	}

	player = characterNew();
	initInventory();
	gtimeSetTime(1<<19);
	checkAutostart();
	#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop(mainloop, 0, true);
	#else
		while(!quit){ mainloop(); }
	#endif

	sfxFreeAll();
	meshFreeAll();
	textureFree();
	shaderFree();
	clientGoodbye();
	clientFree();
	closeSDL();
	closeSingleplayerServer();
	printf("[CLI] Exiting cleanly\n");
	return 0;
}
