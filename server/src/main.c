#define _DEFAULT_SOURCE

#include "main.h"

#include "misc/options.h"
#include "game/blockMining.h"
#include "game/itemDrop.h"
#include "game/grenade.h"
#include "game/blockType.h"
#include "game/recipe.h"
#include "voxel/bigchungus.h"
#include "misc/misc.h"
#include "mods/mods.h"
#include "network/server.h"
#include "tmp/cto.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


const float PI = 3.1415926535897932384626433832795;
bool quit = false;
char *termColors[16];
char *termReset;
char *nop = "";
char *ansiColors[16] = {
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
	for(int i=0;i<16;i++){
		termColors[i] = nop;
	}
	termReset = "";
	if(clicolor == NULL){return;}
	if(*clicolor == 0){return;}
	if(atoi(clicolor) == 0){return;}
	termReset = "\033[0m";
	for(int i=0;i<16;i++){
		termColors[i] = ansiColors[i];
	}
}

void updateWorldStep(){
	blockMiningUpdate();
	itemDropUpdate();
	grenadeUpdate();
}

void updateWorld(){
	static uint64_t lastUpdate = 0;
	if(lastUpdate == 0){lastUpdate = getMillis();}
	for(;lastUpdate+5 < getMillis();lastUpdate+=5){
		updateWorldStep();
	}
}

int main( int argc, const char* argv[] ){
	initSignals();
	initTermColors();
	initOptions(argc,argv);
	seedRNG(time(NULL));


	printf("%sWolkenwelten%s %s %s[%.16s]%s built %s\n",termColors[2],termColors[6],VERSION,termColors[3],COMMIT,termReset,BUILDDATE);
	serverInit();
	bigchungusInit(&world);
	blockTypeInit();
	recipeInit();
	modsInit();
	bigchungusGenSpawn(&world);
	while(!quit){
		bigchungusFreeFarChungi(&world);
		updateWorld();
		serverHandleEvents();
		if(clientCount == 0){
			usleep(100000);
		}else{
			usleep(100);
		}
	}
	printf("Exiting cleanly\n");
	serverFree();
	return 0;
}
