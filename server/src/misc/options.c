#include "options.h"

#include "../main.h"
#include "../../../common/src/tmp/cto.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int   optionWorldSeed    = 0;
int   optionPort         = 0;
bool  optionSingleplayer = false;
bool  verbose            = false;
char  optionSavegame[9];

char *attribution_info =
"LZ4\n"
"https://github.com/lz4/lz4\n"
"\n"
"LZ4 Library\n"
"Copyright (c) 2011-2016, Yann Collet\n"
"All rights reserved.\n"
"\n"
"Redistribution and use in source and binary forms, with or without modification,\n"
"are permitted provided that the following conditions are met:\n"
"\n"
"* Redistributions of source code must retain the above copyright notice, this\n"
"  list of conditions and the following disclaimer.\n"
"\n"
"* Redistributions in binary form must reproduce the above copyright notice, this\n"
"  list of conditions and the following disclaimer in the documentation and/or\n"
"  other materials provided with the distribution.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n"
"ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
"DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR\n"
"ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
"LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON\n"
"ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
"\n"
"----------------------------------------------------------------------------\n"
"\n"
"SHA-1 in C\n"
"By Steve Reid <steve@edmweb.com>\n"
"100% Public Domain\n"
"https://github.com/clibs/sha1\n"
;

void printVersion(){
	printf("%sWolkenwelten Pre-Alpha Dedicated Server%s\n",termColors[2],termReset);
	printf("%sVersion:   %s%s\n",termColors[6],VERSION,termReset);
	printf("%sBuilddate: %s%s\n",termColors[5],BUILDDATE,termReset);
	printf("%sCommit:    %s%s\n",termColors[3],COMMIT,termReset);
	printf("\nAttribution:\n%s\n",attribution_info);
}

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

		if((l = checkString(argv[i]+1,"version"))){
			printVersion();
			exit(0);
			return;
		}
		if((l = checkString(argv[i]+1,"worldSeed="))){
			optionWorldSeed = atoi(argv[i]+l);
			continue;
		}
		if((l = checkString(argv[i]+1,"port="))){
			optionPort = atoi(argv[i]+l);
			continue;
		}
		if((l = checkString(argv[i]+1,"savegame="))){
			strncpy(optionSavegame,argv[i]+l,sizeof(optionSavegame)-1);
			optionSavegame[sizeof(optionSavegame)-1]=0;
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
	if(*optionSavegame == 0){
		snprintf(optionSavegame,sizeof(optionSavegame)-1,"%04X",optionWorldSeed);
		optionSavegame[sizeof(optionSavegame)-1]=0;
	}
	if(optionPort <= 0){
		optionPort = 6309;
	}
}

void initOptions(int argc,const char *argv[]){
	parseOptions(argc,argv);
	sanityCheckOptions();
}
