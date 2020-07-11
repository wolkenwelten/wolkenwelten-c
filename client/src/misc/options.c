#include "options.h"
#include "../tmp/cto.h"
#include "../main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char playerName[28];
char serverName[64];

float optionMusicVolume = 0.75f;
float optionSoundVolume = 1.f;
int   optionWorldSeed   = 0;
bool  optionDebugInfo   = false;

void printVersion(){
	printf("Wolkenwelten Pre-Alpha\n");
	printf("Version:   %s\n",VERSION);
	printf("Builddate: %s\n",BUILDDATE);
	printf("Commit:    %s\n",COMMIT);
	exit(0);
}

void parseOptions(int argc,char *argv[]){
	int tmp;

	for(int i=1;i<argc;i++){
		if(argv[i][0] != '-'){continue;}
		if(strncmp(argv[i]+1,"musicVolume",strlen("musicVolume")) == 0){
			if(argv[i][1+strlen("musicVolume")] == '='){
				tmp = atoi(argv[i]+2+strlen("musicVolume"));
				optionMusicVolume = (float)tmp / 100.f;
			}
		}
		if(strncmp(argv[i]+1,"soundVolume",strlen("soundVolume")) == 0){
			if(argv[i][1+strlen("soundVolume")] == '='){
				tmp = atoi(argv[i]+2+strlen("soundVolume"));
				optionSoundVolume = (float)tmp / 100.f;
			}
		}
		if(strncmp(argv[i]+1,"worldSeed",strlen("worldSeed")) == 0){
			if(argv[i][1+strlen("worldSeed")] == '='){
				optionWorldSeed = atoi(argv[i]+2+strlen("worldSeed"));
			}
		}
		if(strncmp(argv[i]+1,"debugInfo",strlen("debugInfo")) == 0){
			if(argv[i][1+strlen("debugInfo")] == '='){
				optionDebugInfo = atoi(argv[i]+2+strlen("debugInfo")) != 0;
			}
		}
		if(strncmp(argv[i]+1,"playerName",strlen("playerName")) == 0){
			if(argv[i][1+strlen("playerName")] == '='){
				strncpy(playerName,argv[i]+2+strlen("playerName"),sizeof(playerName)-1);
			}
		}
		if(strncmp(argv[i]+1,"serverName",strlen("serverName")) == 0){
			if(argv[i][1+strlen("serverName")] == '='){
				strncpy(serverName,argv[i]+2+strlen("serverName"),sizeof(serverName)-1);
				gameRunning = true;
			}
		}
		if(strncmp(argv[i]+1,"version",strlen("version")) == 0){
			printVersion();
		}
		if(strncmp(argv[i]+1,"-version",strlen("-version")) == 0){
			printVersion();
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
