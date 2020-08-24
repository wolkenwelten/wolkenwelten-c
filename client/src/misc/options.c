#include "options.h"
#include "../../../common/src/tmp/cto.h"
#include "../tmp/assets.h"
#include "../main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char playerName[28];
char serverName[64];

float optionMusicVolume      = 0.75f;
float optionSoundVolume      = 1.f;
int   optionWorldSeed        = 0;
bool  optionDebugInfo        = false;
bool  optionFullscreen       = false;
bool  optionRuntimeReloading = false;

void printVersion(){
	printf("Wolkenwelten Pre-Alpha\n");
	printf("Version:   %s\n",VERSION);
	printf("Builddate: %s\n",BUILDDATE);
	printf("Commit:    %s\n",COMMIT);
	printf("\nAttribution:\n%s\n",txt_attribution_txt_data);
}

int checkString(const char *a, const char *b){
	int len = strlen(b);
	if(strncmp(a,b,len) == 0){return 1+len;}
	if(a[0] == '-' && (strncmp(a+1,b,len) == 0)){return 2+len;}
	return 0;
}

void parseOptions(int argc,char *argv[]){
	int tmp,l;

	for(int i=1;i<argc;i++){
		if(argv[i][0] != '-'){continue;}

		if((l = checkString(argv[i]+1,"musicVolume="))){
			tmp = atoi(argv[i]+l);
			optionMusicVolume = (float)tmp / 100.f;
		}
		if((l = checkString(argv[i]+1,"soundVolume="))){
			tmp = atoi(argv[i]+l);
			optionSoundVolume = (float)tmp / 100.f;
		}
		if((l = checkString(argv[i]+1,"worldSeed="))){
			optionWorldSeed = atoi(argv[i]+l);
		}
		if((l = checkString(argv[i]+1,"debugInfo="))){
			optionDebugInfo = atoi(argv[i]+l) != 0;
		}
		if((l = checkString(argv[i]+1,"playerName="))){
			strncpy(playerName,argv[i]+l,sizeof(playerName)-1);
		}
		if((l = checkString(argv[i]+1,"serverName="))){
			strncpy(serverName,argv[i]+l,sizeof(serverName)-1);
			gameRunning = true;
		}

		if(checkString(argv[i]+1,"windowed")){
			optionFullscreen = false;
		}
		if(checkString(argv[i]+1,"fullscreen")){
			optionFullscreen = true;
		}
		if(checkString(argv[i]+1,"runtimeReloading")){
			optionRuntimeReloading = true;
		}
		if(checkString(argv[i]+1,"version")){
			printVersion();
			exit(0);
		}
	}
}

void sanityCheckOptions(){
	if(optionMusicVolume < 0.f){
		optionMusicVolume = 0.f;
		printf("Using Minimum musicVolume of 0\n");
	}
	if(optionMusicVolume > 1.f){
		optionMusicVolume = 1.f;
		printf("Using Maximum musicVolume of 100\n");
	}
	if(optionSoundVolume < 0.f){
		optionSoundVolume = 0.f;
		printf("Using Minimum soundVolume of 0\n");
	}
	if(optionSoundVolume > 1.f){
		optionSoundVolume = 1.f;
		printf("Using Maximum soundVolume of 100\n");
	}
	if(serverName[0] == 0){
		strncpy(serverName,"localhost",sizeof(serverName)-1);
	}
	if(optionWorldSeed != 0){
		singleplayer = true;
		gameRunning  = true;
	}
}

void initOptions(int argc,char *argv[]){
	parseOptions(argc,argv);
	sanityCheckOptions();
}
