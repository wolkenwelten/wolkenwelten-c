#include "main.h"

#include "gfx/clouds.h"
#include "gfx/sky.h"
#include "gfx/gfx.h"
#include "gfx/mesh.h"
#include "gfx/shader.h"
#include "gfx/particle.h"
#include "gfx/texture.h"
#include "gui/gui.h"
#include "gui/menu.h"
#include "gui/inventory.h"
#include "gui/overlay.h"
#include "game/animal.h"
#include "game/blockType.h"
#include "game/blockMining.h"
#include "game/entity.h"
#include "game/itemDrop.h"
#include "game/grenade.h"
#include "game/recipe.h"
#include "misc/options.h"
#include "network/client.h"
#include "network/chat.h"
#include "sdl/sdl.h"
#include "sdl/sfx.h"
#include "sdl/input_mouse.h"
#include "sdl/input_keyboard.h"
#include "sdl/input_gamepad.h"
#include "sdl/input_touch.h"
#include "tmp/objs.h"
#include "voxel/chungus.h"
#include "voxel/bigchungus.h"
#include "../../common/src/misc/misc.h"
#include "../../common/src/mods/mods.h"

#include <time.h>
#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#endif

bool quit              = false;
bool gameRunning       = false;
bool playerChunkActive = false;
bool singleplayer      = false;

void playerUpdate(){
	vec nv = vecZero();
	chungus *chng = worldGetChungus((int)player->pos.x >> 8,(int)player->pos.y >> 8,(int)player->pos.z >> 8);
	if(chng != NULL){ playerChunkActive = chng->loaded; }
	if(player->flags & CHAR_SPAWNING){return;}
	if(!playerChunkActive){return;}
	if(inputSneak()){
		player->flags |= CHAR_SNEAK;
	}else{
		player->flags &= ~CHAR_SNEAK;
	}

	nv = doKeyboardupdate(nv);
	nv = doTouchupdate   (nv);
	nv = doGamepadupdate (nv);
	characterMove (player,nv);
}

void worldUpdate(){
	static int lastTick=0;
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
		if(isInventoryOpen()){setOverlayColor(0x80000000,300);}
		charactersUpdate();
		commitOverlayColor();
		grenadeUpdate();
		animalUpdateAll();
		entityUpdateAll();
		itemDropUpdate();
		particleUpdate();
	}
}

void mainloop(){
	handleEvents();
	if(gameRunning){
		clientHandleEvents();
		playerUpdate();
		worldUpdate();
		renderFrame();
		bigchungusFreeFarChungi(&world,player);
		chatCheckInput();
		clientSendAllToServer();
	}else{
		doGamepadMenuUpdate();
		renderMenuFrame();
	}
}

int main( int argc, char* argv[] ){
	clientGetName();
	initOptions(argc,argv);
	initSDL();
	seedRNG(time(NULL));

	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	bigchungusInit(&world);
	shaderInit();
	textureInit();
	sfxInit();

	initUI();
	initMenu();
	initSky();
	cloudsInit();
	blockTypeInit();
	blockMiningInit();
	initMeshobjs();
	particleInit();
	recipeInit();

	modsInit();
	textureBuildBlockIcons(0);

	if(gameRunning){
		hideMouseCursor();
	}else{
		showMouseCursor();
	}


	player = characterNew();
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
	return 0;
}
