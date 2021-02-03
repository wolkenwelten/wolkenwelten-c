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
#include "character.h"

#include "../game/character.h"
#include "../misc/options.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/network/messages.h"
#include "../../../common/src/misc/misc.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>

static void characterParseDataLine(character *p, const char *line){
	int argc;
	char **argv;

	argv = splitArgs(line,&argc);
	if(argc == 0)          {return;}
	if(argv[0][0] == 0)    {return;}
	if(isspace((u8)argv[0][0])){return;}

	if(strcmp(argv[0],"Position") == 0){
		if(argc < 7){return;}
		p->pos = vecNew(atof(argv[1]),atof(argv[2]),atof(argv[3]));
		p->rot = vecNew(atof(argv[4]),atof(argv[5]),atof(argv[6]));
		if(!inWorld(p->pos.x,p->pos.y,p->pos.z)){p->pos = vecAdd(vecNewI(worldGetSpawnPos()),vecNew(.5f,2.f,.5f));}
		return;
	}

	if(strcmp(argv[0],"Health") == 0){
		if(argc < 2){return;}
		p->hp = atoi(argv[1]);
		return;
	}

	if(strcmp(argv[0],"Flags") == 0){
		if(argc < 2){return;}
		p->flags = atoi(argv[1]);
		return;
	}

	if(strcmp(argv[0],"ActiveItem") == 0){
		if(argc < 2){return;}
		p->activeItem = atoi(argv[1]);
		return;
	}

	if(strcmp(argv[0],"Item") == 0){
		if(argc < 4){return;}
		uint i = atoi(argv[1]);
		if(i >= 40){return;}
		p->inventory[i].ID     = atoi(argv[2]);
		p->inventory[i].amount = atoi(argv[3]);
		return;
	}

	if(strcmp(argv[0],"Equipment") == 0){
		if(argc < 4){return;}
		uint i = atoi(argv[1]);
		if(i >= 3){return;}
		p->equipment[i].ID     = atoi(argv[2]);
		p->equipment[i].amount = atoi(argv[3]);
		return;
	}
}

static const char *characterFileName(const char *name){
	static char filename[128];
	snprintf(filename,sizeof(filename)-1,"save/%s/%s.player",optionSavegame,name);
	filename[sizeof(filename)-1] = 0;
	return filename;
}

static void characterSendData(const character *p, uint c){
	msgPlayerSetPos(c,p->pos,p->rot);
	msgPlayerSetInventory(c,p->inventory,40);
	msgPlayerSetEquipment(c,p->equipment, 3);

	msgPlayerSetData(c,p->hp,p->activeItem,p->flags, c);
}

static int characterLoadData(character *p, const char *pName){
	size_t len = 0;
	const char *filename;
	char *b,*line;
	#ifdef __EMSCRIPTEN__
	return 0;
	#endif
	filename = characterFileName(pName);
	b = loadFile(filename,&len);
	if((b == NULL) || (len == 0)){return 0;}

	line = b;
	for(unsigned int i=0;i<len;i++){
		if(b[i] == '\r'){b[i] = 0;}
		if(b[i] == '\n'){
			b[i] = 0;
			characterParseDataLine(p,line);
			line = &b[i+1];
		}
	}
	characterParseDataLine(p,line);

	return 1;
}

void characterSaveData(const character *p, const char *pName){
	static char buf[8192];
	char *b;
	#ifdef __EMSCRIPTEN__
	return;
	#endif
	if(p == NULL){return;}
	if(pName == NULL){return;}
	if(*pName == 0){return;}

	b = buf;
	b += snprintf(b,sizeof(buf)-(b-buf+1),"Position %f %f %f %f %f %f\n",p->pos.x,p->pos.y,p->pos.z,p->rot.yaw,p->rot.pitch,p->rot.roll);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"ActiveItem %i\n",p->activeItem);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"Health %i\n",p->hp);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"Flags %u\n",p->flags);

	for(uint i=0;i<40;i++){
		if(itemIsEmpty(&p->inventory[i])){continue;}
		b += snprintf(b,sizeof(buf)-(b-buf+1),"Item %u %u %i\n",i,p->inventory[i].ID,p->inventory[i].amount);
	}
	for(uint i=0;i<3;i++){
		if(itemIsEmpty(&p->equipment[i])){continue;}
		b += snprintf(b,sizeof(buf)-(b-buf+1),"Equipment %u %u %i\n",i,p->equipment[i].ID,p->equipment[i].amount);
	}

	*b = 0;
	buf[sizeof(buf)-1]=0;
	saveFile(characterFileName(pName),buf,strlen(buf));
}

void characterLoadSendData(character *p, const char *pName, uint c){
	if(p == NULL){return;}
	if(!characterLoadData(p,pName)){
		const vec spawn = vecNewI(worldGetSpawnPos());
		p->pos = vecAdd(spawn,vecNew(.5f,2.f,.5f));
		p->rot = vecNew(135.f,15.f,0.f);
	}
	characterSendData(p,c);
}
