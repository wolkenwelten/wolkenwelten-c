#include "options.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int   optionWorldSeed    = 0;
int   optionPort         = 0;
bool  optionSingleplayer = false;
bool  verbose            = false;

int checkString(const char *a, const char *b){
	int len = strlen(b);
	if(strncmp(a,b,len) == 0){return 1+len;}
	if(a[0] == '-' && (strncmp(a+1,b,len) == 0)){return 2+len;}
	return 0;
}

void parseOptions(int argc,const char *argv[]){
	int l;
	for(int i=0;i<argc;i++){
		if(argv[i][0] != '-'){continue;}

		if((l = checkString(argv[i]+1,"worldSeed="))){
			optionWorldSeed = atoi(argv[i]+l);
			continue;
		}
		if((l = checkString(argv[i]+1,"port="))){
			optionPort = atoi(argv[i]+l);
			continue;
		}
		if((l = checkString(argv[i]+1,"verbose"))){
			verbose = true;
			continue;
		}
		if((l = checkString(argv[i]+1,"singleplayer="))){
			if(atoi(argv[i]+l) != 0){
				optionSingleplayer = true;
				continue;
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
