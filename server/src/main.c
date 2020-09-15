#define _DEFAULT_SOURCE

#include "main.h"

#include "misc/options.h"
#include "game/animal.h"
#include "game/blockMining.h"
#include "game/itemDrop.h"
#include "game/grenade.h"
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

uint64_t getMillis(){
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
			termColors[i] = ansiColors[i];
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
	static uint64_t lastUpdate = 0;
	static uint64_t lastThought = 0;
	if(lastUpdate == 0){lastUpdate = getMillis();}
	for(;lastUpdate+5 < getMillis();lastUpdate+=5){
		updateWorldStep();
	}

	if(lastThought == 0){lastThought = getMillis() - 500;}
	for(;lastThought+500 < getMillis();lastThought+=500){
		thinkStep();
	}
}

void checkValidSavegame(const char *name){
	char buf[16];
	if(!isDir("save")){makeDir("save");}
	snprintf(buf,sizeof(buf)-1,"save/%s",name);
	buf[sizeof(buf)-1] = 0;
	if(!isDir(buf)){makeDir(buf);}
}

int main( int argc, const char* argv[] ){
	initSignals();
	initTermColors();
	initOptions(argc,argv);
	checkValidSavegame(optionSavegame);
	seedRNG(time(NULL));

	printf("%sWolkenwelten",termColors[2]                );
	printf(" %s%s"         ,termColors[6],VERSION        );
	printf(" %s[%.16s]"    ,termColors[3],COMMIT         );
	printf(" %sSeed[%u]"   ,termColors[4],optionWorldSeed);
	printf(" %s{%s}"       ,termColors[5],optionSavegame );
	printf(" %sbuilt %s\n" ,termReset    ,BUILDDATE      );

	serverInit();
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
	serverFree();
	return 0;
}
