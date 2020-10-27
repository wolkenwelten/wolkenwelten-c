#include "main.h"

#include "misc/options.h"
#include "game/animal.h"
#include "game/blockMining.h"
#include "game/itemDrop.h"
#include "game/grenade.h"
#include "persistence/savegame.h"
#include "voxel/bigchungus.h"
#include "network/server.h"
#include "../../common/src/tmp/cto.h"
#include "../../common/src/misc/misc.h"
#include "../../common/src/game/blockType.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

bool quit = false;
char *termColors[16];
char *termReset;
char *nop = "";
char *ansiFG[16] = {
	"\033[0;30m",
	"\033[0;31m",
	"\033[0;32m",
	"\033[0;33m",
	"\033[0;34m",
	"\033[0;35m",
	"\033[0;36m",
	"\033[0;37m",
	"\033[1;30m",
	"\033[1;31m",
	"\033[1;32m",
	"\033[1;33m",
	"\033[1;34m",
	"\033[1;35m",
	"\033[1;36m",
	"\033[1;37m"
};

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

void initTermColors(){
	char *clicolor = getenv("CLICOLOR");
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	if((clicolor == NULL) || (*clicolor == 0) || (atoi(clicolor) == 0)){
		termReset = "";
		for(int i=0;i<16;i++){
			termColors[i] = nop;
		}
	}else{
		termReset = "\033[0m";
		for(int i=0;i<16;i++){
			termColors[i] = ansiFG[i];
		}
	}
}

void updateWorldStep(){
	blockMiningUpdate();
	itemDropUpdate();
	grenadeUpdate();
	animalUpdateAll();
}

void thinkStep(){
	animalThinkAll();
}

void updateWorld(){
	static u64 lastUpdate  = 0;
	static u64 lastThought = 0;
	if(lastUpdate  == 0){lastUpdate  = getTicks() -   5;}
	if(lastThought == 0){lastThought = getTicks() - 100;}

	for(;lastUpdate +  5 < getTicks();lastUpdate +=  5){
		updateWorldStep();
	}
	for(;lastThought+100 < getTicks();lastThought+=100){
		thinkStep();
	}
}

int main( int argc, const char* argv[] ){
	initSignals();
	initTermColors();
	initOptions(argc,argv);
	savegameLoad();
	seedRNG(time(NULL));
	serverInit();
	savegameSave();

	printf("%sWolkenwelten",termColors[2]                );
	printf(" %s%s"         ,termColors[6],VERSION        );
	printf(" %s[%.16s]"    ,termColors[3],COMMIT         );
	printf(" %sSeed[%u]"   ,termColors[4],optionWorldSeed);
	printf(" %s{%s}"       ,termColors[5],optionSavegame );
	printf(" %sbuilt %s\n" ,termReset    ,BUILDDATE      );

	bigchungusInit(&world);
	blockTypeInit();
	bigchungusGenSpawn(&world);
	while(!quit){
		bigchungusFreeFarChungi(&world);
		bigchungusSafeSave(&world);
		updateWorld();
		serverHandleEvents();
		if(clientCount == 0){
			usleep(100000);
		}else{
			usleep(1000);
		}
	}
	printf("Exiting cleanly\n");
	savegameSave();
	serverFree();
	return 0;
}
