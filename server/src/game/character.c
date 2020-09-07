#include "character.h"

#include "../main.h"
#include "../misc/options.h"
#include "../game/itemDrop.h"
#include "../game/blockMining.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

character characterList[128];
int characterCount = 0;
character *characterFirstFree = NULL;

void characterInit(character *c){
	int sx,sy,sz;

	c->gyoff  = 0.f;
	c->gvx = c->gvy = c->gvz = 0.f;

	c->flags = 0;
	c->gliderFade = 0.f;
	c->animationIndex = c->animationTicksMax = c->animationTicksLeft = 0;

	c->actionTimeout = 0;
	c->stepTimeout   = 0;
	c->activeItem    = 0;

	c->blockMiningX  = c->blockMiningY = c->blockMiningZ = -1;
	c->hp = c->maxhp = 20;

	bigchungusGetSpawnPos(&world,&sx,&sy,&sz);
	c->yaw   = 135.f;
	c->pitch = 0.f;
	c->roll  = 0.f;
	c->yoff  = 0.f;
	c->x     = sx+0.5f;
	c->y     = sy+1.0f;
	c->z     = sz+0.5f;
	c->vx    = c->vy = c->vz = 0.f;
	c->hook  = false;
	c->hookx = c->hooky = c->hookz = 0.f;

	characterEmptyInventory(c);
}

character *characterNew(){
	character *c = NULL;
	if(characterFirstFree != NULL){
		c = characterFirstFree;
		characterFirstFree = c->nextFree;
	}
	if(c == NULL){
		if(characterCount >= (int)(sizeof(characterList) / sizeof(character))-1){
			fprintf(stderr,"characterList Overflow!\n");
			return NULL;
		}
		c = &characterList[characterCount++];
	}
	c->hook = NULL;
	characterInit(c);

	return c;
}

void characterFree(character *c){
	c->nextFree = characterFirstFree;
	characterFirstFree = c;
}

void characterDie(character *c){
	characterInit(c);
}

void characterParseDataLine(int c,const char *line){
	int argc;
	char **argv;
	
	argv = splitArgs(line,&argc);
	if(argc == 0)          {return;}
	if(argv[0][0] == 0)    {return;}
	if(isspace(argv[0][0])){return;}
	
	if(strcmp(argv[0],"Position") == 0){
		if(argc < 7){return;}
		msgPlayerSetPos(c,atof(argv[1]),atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5]),atof(argv[6]));
		return;
	}
	
	if(strcmp(argv[0],"Health") == 0){
		if(argc < 2){return;}
		msgPlayerSetData(c,atoi(argv[1]));
		return;
	}
}

char *characterFileName(const char *name){
	static char filename[128];
	snprintf(filename,sizeof(filename)-1,"save/%s/%s.player",optionSavegame,name);
	filename[sizeof(filename)-1] = 0;
	return filename;
}

void characterSaveData(int c){
	static char buf[4096];
	char *b;
	if(clients[c].c == NULL){return;}
	
	b = buf;
	b += snprintf(b,sizeof(buf)-(b-buf+1),"Position %f %f %f %f %f %f\n",clients[c].c->x,clients[c].c->y,clients[c].c->z,clients[c].c->yaw,clients[c].c->pitch,clients[c].c->roll);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"Health %i\n",clients[c].c->hp);
	
	*b = 0;
	buf[sizeof(buf)-1]=0;
	saveFile(characterFileName(clients[c].playerName),buf,strlen(buf));
}

int characterLoadData(int c){
	size_t len = 0;
	char *filename,*b,*line;
	filename = characterFileName(clients[c].playerName);
	b = loadFile(filename,&len);
	if((b == NULL) || (len == 0)){return 0;}
	
	line = b;
	for(unsigned int i=0;i<len;i++){
		if(b[i] == '\r'){b[i] = 0;}
		if(b[i] == '\n'){
			b[i] = 0;
			characterParseDataLine(c,line);
			line = &b[i+1];
		}
	}
	characterParseDataLine(c,line);
	
	return 1;
}

void characterLoadSendData(int c){
	int sx,sy,sz;
	if(characterLoadData(c)){return;}
	
	bigchungusGetSpawnPos(&world,&sx,&sy,&sz);
	msgPlayerSetPos(c,((float)sx)+0.5f,((float)sy)+2.f,((float)sz)+0.5f,0.f,0.f,0.f);
}
