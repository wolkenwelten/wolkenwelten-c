#include "main.h"

#include "misc/options.h"
#include "sdl/sdl.h"
#include "sdl/sfx.h"
#include "gui/gui.h"
#include "gui/menu.h"
#include "gfx/sky.h"
#include "gfx/gfx.h"
#include "gfx/mesh.h"
#include "gfx/shader.h"
#include "gfx/particle.h"
#include "gfx/texture.h"
#include "gui/inventory.h"
#include "game/entity.h"
#include "game/itemDrop.h"
#include "game/grenade.h"
#include "game/blockType.h"
#include "game/blockMining.h"
#include "game/recipe.h"
#include "voxel/chungus.h"
#include "voxel/bigchungus.h"
#include "gfx/objs.h"
#include "../../common/src/misc/misc.h"
#include "../../common/src/mods/mods.h"
#include "network/client.h"
#include "network/chat.h"

#include "sdl/input_mouse.h"
#include "sdl/input_keyboard.h"
#include "sdl/input_gamepad.h"
#include "sdl/input_touch.h"
#include "sdl/sdl.h"

#include <stdbool.h>
#include <time.h>
#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#endif

bool quit              = false;
bool gameRunning       = false;
bool playerChunkActive = false;
bool singleplayer      = false;

void playerUpdate(){
	static int lastTick=0;
	int curTick;
	float vx=0.f;
	float vz=0.f;
	float vy=0.f;
	chungus *chng = worldGetChungus((int)player->x >> 8,(int)player->y >> 8,(int)player->z >> 8);
	if(chng != NULL){ playerChunkActive = chng->loaded; }
	if(!playerChunkActive){return;}
	player->sneak = inputSneak();

	doKeyboardupdate(&vx,&vy,&vz);
	doTouchupdate   (&vx,&vy,&vz);
	doGamepadupdate (&vx,&vy,&vz);
	characterMove   (player,vx,vy,vz);

	if(lastTick == 0){lastTick = SDL_GetTicks();}
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
		characterUpdate(player);
		commitOverlayColor();
		grenadeUpdate();
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
