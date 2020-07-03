#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "options.h"

int   optionWorldSeed    = 0;
int   optionPort         = 0;
bool  optionSingleplayer = false;
bool  verbose            = false;

void parseOptions(int argc,const char *argv[]){
	for(int i=0;i<argc;i++){
		if(argv[i][0] != '-'){continue;}
		if(strncmp(argv[i]+1,"worldSeed",strlen("worldSeed")) == 0){
			if(argv[i][1+strlen("worldSeed")] == '='){
				optionWorldSeed = atoi(argv[i]+2+strlen("worldSeed"));
				continue;
			}
		}
		if(strncmp(argv[i]+1,"port",strlen("port")) == 0){
			if(argv[i][1+strlen("port")] == '='){
				optionPort = atoi(argv[i]+2+strlen("port"));
				continue;
			}
		}
		if(strncmp(argv[i]+1,"verbose",strlen("verbose")) == 0){
			verbose = true;
			continue;
		}
		if(strncmp(argv[i]+1,"singleplayer",strlen("singleplayer")) == 0){
			if(argv[i][1+strlen("singleplayer")] == '='){
				if(atoi(argv[i]+2+strlen("singleplayer")) != 0){
					optionSingleplayer = true;
					continue;
				}
			}
		}
		break;
	}
}

void sanityCheckOptions(){
	if(optionWorldSeed == 0){
		optionWorldSeed = (int)(time(NULL)&0xFFFF);
	}
	if(optionPort <= 0){
		optionPort = 6309;
	}
}

void initOptions(int argc,const char *argv[]){
	parseOptions(argc,argv);
	sanityCheckOptions();
}
