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

#include "misc/lisp.h"
#include "misc/options.h"
#include "game/animal.h"
#include "game/being.h"
#include "game/blockMining.h"
#include "game/fire.h"
#include "game/itemDrop.h"
#include "game/landscape.h"
#include "game/grenade.h"
#include "game/throwable.h"
#include "game/time.h"
#include "game/rain.h"
#include "game/weather.h"
#include "persistence/savegame.h"
#include "voxel/bigchungus.h"
#include "voxel/chungus.h"
#include "voxel/chunk.h"
#include "network/server.h"

#include "../../common/src/nujel/nujel.h"
#include "../../common/src/nujel/string.h"
#include "../../common/src/tmp/cto.h"
#include "../../common/src/misc/misc.h"
#include "../../common/src/misc/profiling.h"
#include "../../common/src/misc/test.h"
#include "../../common/src/mods/mods.h"
#include "../../common/src/game/blockType.h"
#include "../../common/src/game/projectile.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

int playerID  = 64;
bool isClient = false;
bool quit = false;
char *termColors[16];
char *termReset;
char *nop = "";

u64 getTicks(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
}

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

static void initTermColors(){
	char *clicolor = getenv("CLICOLOR");
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	if((clicolor == NULL) || (*clicolor == 0) || (atoi(clicolor) == 0)){
		termReset = "";
		for(int i=0;i<16;i++){
			termColors[i] = nop;
		}
	}else{
		termReset = ansiRS;
		for(int i=0;i<16;i++){
			termColors[i] = ansiFG[i];
		}
	}
}

static void updateWorldStep(){
	blockMiningUpdateAll();
	itemDropUpdateAll();
	grenadeUpdateAll();
	animalUpdateAll();
	projectileUpdateAll();
	animalThinkAll();
	fireUpdateAll();
	animalNeedsAll();
	animalCheckBurnAll();
	landscapeUpdateAll();
	weatherUpdateAll();
	rainUpdateAll();
	throwableUpdateAll();

	gtimeUpdate();
}

int updateWorld(){
	static u64 lastUpdate  = 0;
	const u64 cTicks = getTicks();
	if(lastUpdate  == 0){lastUpdate  = getTicks() - msPerTick;}

	int i = 64;
	for(;lastUpdate < cTicks;lastUpdate += msPerTick){
		updateWorldStep();
		if(--i == 0){break;}
	}
	lispEvents();
	if(lastUpdate < cTicks){return 0;}
	return 1;
}

void handleAnimalPriorities(){
	static u64 lastCall   = 0;
	static uint c = 0;
	const u64 cTicks = getTicks();
	if(cTicks < lastCall + msPerTick*128) {return;}
	c = (c+1) & 0x1F;
	if(clients[c].state)        {return;}
	if(clients[c].c == NULL)    {return;}
	lastCall += msPerTick*128;
	animalUpdatePriorities(c);
}

void mainTick(){
	freeTime = getTicks();
	chungusUnsubFarChungi();
	chungusFreeOldChungi(30000);
	handleAnimalPriorities();
	bigchungusSafeSave(&world,false);
	playerSafeSave();
	serverHandleEvents();
	updateWorld();
	serverHandleEvents();
}

void mainInit(){
	gtimeSetTime(1<<19);
	seedRNG(time(NULL));
	weatherInit();
	savegameLoad();
	serverInit();
	beingListEntryInit();
	chungusInit();
	chunkInit();
	lispInit();
	modsInit();
	savegameSave();

	bigchungusInit(&world);
	blockTypeInit();
	if(optionProfileWG){
		bigchungusGenHugeSpawn(&world);
	}else{
		bigchungusGenSpawn(&world);
	}
}

static void checkSPQuit(){
	static u64 startTime = 0;
	if(!optionSingleplayer){return;}
	if(startTime == 0){startTime = getTicks();}
	if(clientCount > 0){return;}
	if(getTicks() > startTime + 1000){quit = true;}
}

#ifndef __EMSCRIPTEN__
int main( int argc, const char* argv[] ){
	selfTest();
	initSignals();
	initTermColors();
	initOptions(argc,argv);
	changeToDataDir(); // Change to data dir after parsing args so we can add an argument later to set the data dir
	mainInit();

	printf("%sWolkenwelten",termColors[2]                );
	printf(" %s%s"         ,termColors[6],VERSION        );
	printf(" %s[%.16s]"    ,termColors[3],COMMIT         );
	printf(" %sSeed[%u]"   ,termColors[4],optionWorldSeed);
	printf(" %s{%s}"       ,termColors[5],optionSavegame );
	printf(" %sbuilt %s\n" ,termReset    ,BUILDDATE      );

	while(!quit){
		if(optionProfileWG){quit = true;}
		if(clientCount == 0){checkSPQuit();}
		mainTick();
		usleep(100);
	}

	printf("[SRV] Exiting cleanly\n");
	savegameSave();
	bigchungusSafeSave(&world,true);
	serverFree();

	fflush(stdout);
	fflush(stderr);
	return 0;
}
#endif
