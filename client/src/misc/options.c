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

#include "options.h"

#include "../main.h"
#include "../gui/menu.h"
#include "../gfx/gfx.h"
#include "../tmp/assets.h"
#include "../misc/lisp.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/tmp/cto.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char playerName[28];
char serverName[64];
char optionSavegame[32];

float optionMusicVolume       = 0.25f;
float optionSoundVolume       = 0.5f;
int   optionAutomatedTest     = 0;
int   optionWorldSeed         = 0;
bool  optionDebugInfo         = false;
bool  optionFullscreen        = false;
bool  optionRuntimeReloading  = false;
bool  optionNoSave            = false;
bool  optionMute              = false;
bool  optionThirdPerson       = false;
int   optionWindowOrientation = 0;
int   optionWindowWidth       = 0;
int   optionWindowHeight      = 0;
bool  optionWireframe         = false;

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
		if((l = checkString(argv[i]+1,"mute="))){
			optionMute = atoi(argv[i]+l) != 0;
		}
		if((l = checkString(argv[i]+1,"playerName="))){
			strncpy(playerName,argv[i]+l,sizeof(playerName)-1);
		}
		if((l = checkString(argv[i]+1,"serverName="))){
			strncpy(serverName,argv[i]+l,sizeof(serverName)-1);
		}
		if((l = checkString(argv[i]+1,"savegame="))){
			strncpy(optionSavegame,argv[i]+l,sizeof(optionSavegame)-1);
			optionSavegame[sizeof(optionSavegame)-1]=0;
		}
		if((l = checkString(argv[i]+1,"automatedTest="))){
			optionAutomatedTest = atoi(argv[i]+l);
		}
		if((l = checkString(argv[i]+1,"windowOrientation="))){
			int newOrientation = 0;
			if(argv[i][l] != 0){
				u8 chr = toupper((u8)argv[i][l]);
				if(chr == 'T'){newOrientation |= 0x10;}
				if(chr == 'B'){newOrientation |= 0x20;}
				if(chr == 'L'){newOrientation |= 0x01;}
				if(chr == 'R'){newOrientation |= 0x02;}
				if(argv[i][l+1] != 0){
					chr = toupper((u8)argv[i][l+1]);
					if(chr == 'T'){newOrientation |= 0x10;}
					if(chr == 'B'){newOrientation |= 0x20;}
					if(chr == 'L'){newOrientation |= 0x01;}
					if(chr == 'R'){newOrientation |= 0x02;}
				}
			}
			optionWindowOrientation = newOrientation;
		}
		if((l = checkString(argv[i]+1,"windowWidth="))){
			optionWindowWidth = atoi(argv[i]+l);
		}
		if((l = checkString(argv[i]+1,"windowHeight="))){
			optionWindowHeight = atoi(argv[i]+l);
		}

		if(checkString(argv[i]+1,"windowed")){
			optionFullscreen = false;
		}
		if(checkString(argv[i]+1,"fullscreen")){
			optionFullscreen = true;
		}
		if(checkString(argv[i]+1,"noSave")){
			optionNoSave = true;
		}
		if(checkString(argv[i]+1,"version")){
			printVersion();
			exit(0);
		}
		if(checkString(argv[i]+1,"thirdPerson")){
			optionThirdPerson = true;
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
	if(optionWindowWidth <= 0){
		optionWindowWidth = 800;
	}
	if(optionWindowHeight <= 0){
		optionWindowHeight = 480;
	}
	if(optionAutomatedTest < 0){
		optionAutomatedTest = 0;
	}
}

void loadOptionFile(const char *fname){
	size_t len = 0;
	char *b = loadFile(fname,&len);
	if((b == NULL) || (len == 0)){return;}
	lispEval(b);
	free(b);
}

void loadOptions(){
	#ifdef __EMSCRIPTEN__
	return;
	#endif
	if(optionNoSave){return;}
	loadOptionFile("client.nuj");
	loadOptionFile("custom.nuj");
}

void saveOptions(){
	static char buf[4096];
	char *b;
	#ifdef __EMSCRIPTEN__
	return;
	#endif
	if(optionNoSave){return;}

	b  = buf;
	b += snprintf(b,sizeof(buf)-(b-buf+1),";; This file gets automatically overwritten by the client.\n;; Do not edit this file, your changes will get lost!\n;; \n;; Instead, write everything into custom.nuj which the client will never change, only read and eval.\n\n");
	b += snprintf(b,sizeof(buf)-(b-buf+1),"(conf-v1!\n");
	b += snprintf(b,sizeof(buf)-(b-buf+1),"  (player-name!  \"%s\")\n",playerName);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"  (sound-vol!    %f)\n",optionSoundVolume);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"  (view-dist!    %f)\n",renderDistance);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"  (fullscreen!   %s)\n",optionFullscreen ? "#t" : "#f");
	b += snprintf(b,sizeof(buf)-(b-buf+1),"  (third-person! %s)\n",optionThirdPerson ? "#t" : "#f");
	b += snprintf(b,sizeof(buf)-(b-buf+1),"  (debug-info!   %s)",optionDebugInfo ? "#t" : "#f");
	for(int i=0;i<serverlistCount;i++){
		b += snprintf(b,sizeof(buf)-(b-buf+1),"\n  (server-add! \"%s\" \"%s\")",serverlistIP[i],serverlistName[i]);
	}
	b += snprintf(b,sizeof(buf)-(b-buf+1),")\n\n");

	buf[sizeof(buf)-1] = 0;
	saveFile("client.nuj",buf,strlen(buf));
}

void initOptions(int argc,char *argv[]){
	loadOptions();
	parseOptions(argc,argv);
	sanityCheckOptions();
}
