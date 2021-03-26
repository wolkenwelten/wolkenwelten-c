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
#include "savegame.h"

#include "animal.h"
#include "character.h"
#include "chunk.h"
#include "fire.h"
#include "itemDrop.h"
#include "throwable.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../misc/options.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/time.h"
#include "../../../common/src/game/weather.h"
#include "../../../common/src/misc/lz4.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u8 *saveLoadBuffer   = NULL;
u8 *compressedBuffer = NULL;

static void bigchungusSafeSaveClient(const bigchungus *c, const character *chara){
	if(chara == NULL){return;}
	ivec cp = ivecNewV(chara->pos);
	if((cp.x >= 0) && (cp.x < 256) && (cp.y >= 0) && (cp.y < 128) && (cp.z >= 0) && (cp.z < 256)){
		chungusSave(c->chungi[cp.x][cp.y][cp.z]);
	}
	for(int ix=0;ix < 12; ix++){
		for(int iy=0;iy < 12; iy++){
			for(int iz=0;iz < 12; iz++){

				int ox = ix >> 1;
				if(ix & 1){ox = -ox;}
				ox = cp.x+ox;
				if(ox <   0){goto xcontinue;}
				if(ox > 255){goto xcontinue;}

				int oy = iy >> 1;
				if(iy & 1){oy = -oy;}
				oy = cp.y+oy;
				if(oy <   0){goto ycontinue;}
				if(oy > 127){goto ycontinue;}

				int oz = iz >> 1;
				if(iz & 1){oz = -oz;}
				oz = cp.z+oz;
				if(oz <   0){continue;}
				if(oz > 255){continue;}

				chungusSave(c->chungi[ox][oy][oz]);
			}
			ycontinue: (void)c;
		}
		xcontinue: (void)c;
	}
}

static const char *chungusGetFilename(chungus *c){
	static char buf[64];
	snprintf(buf,sizeof(buf),"save/%s/%02X%02X%02X.chunk",optionSavegame,c->x,c->y,c->z);
	return buf;
}

static const char *savegameFileName(const char *name){
	static char filename[128];
	snprintf(filename,sizeof(filename),"save/%s/world.global",name);
	return filename;
}

static void checkValidSavegame(const char *name){
	static char buf[128];
	if(!isDir("save")){makeDir("save");}
	snprintf(buf,sizeof(buf),"save/%.120s",name);
	if(!isDir(buf)){makeDir(buf);}
}

void bigchungusSafeSave(const bigchungus *c, bool force){
	static u64 lastSave = 0;
	if((!force) && (getTicks() < lastSave+60000)){return;}
	lastSave = getTicks();

	for(uint i=0;i < chungusCount; i++){
		chungusSave(&chungusList[i]);
	}

	for(uint i=0;i<clientCount;i++){
		if(clients[i].state == 2){ continue; }
		if(clients[i].c == NULL ){ continue; }
		bigchungusSafeSaveClient(c,clients[i].c);
	}
}

void chungusLoad(chungus *c){
	#ifdef __EMSCRIPTEN__
	return;
	#endif
	if(c == NULL)               { return; }
	if(saveLoadBuffer == NULL)  { saveLoadBuffer   = malloc(4100*4096); }
	if(compressedBuffer == NULL){ compressedBuffer = malloc(4100*4096); }
	size_t read=0,len=0;
	const u8 *b,*end;
	int i;

	FILE *fp = fopen(chungusGetFilename(c),"rb");
	if(fp == NULL){return;}
	fseek(fp,0,SEEK_END);
	len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	for(i=0;i<64;i++){
		read += fread(compressedBuffer+read,1,len-read,fp);
		if(read >= len){break;}
	}
	if(i==64){
		fprintf(stderr,"Error reading chungus %i:%i:%i\n",c->x>>8,c->y>>8,c->z>>8);
		return;
	}

	len = LZ4_decompress_safe((const char *)compressedBuffer, (char *)saveLoadBuffer, len, 4100*4096);
	end = &saveLoadBuffer[len];
	animalDelChungus(c);

	for(b=saveLoadBuffer;b<end;){
		saveType cType = *b;
		switch(cType){
		case saveTypeChunk:
			b = chunkLoad(c,b);
			break;
		case saveTypeItemDrop:
			b = itemDropLoad(b);
			break;
		case saveTypeAnimal:
			b = animalLoad(b);
			break;
		case saveTypeFire:
			b = fireLoad(b);
			break;
		case saveTypeThrowable:
			b = throwableLoad(b);
			break;
		default:
			fprintf(stderr,"Unknown type[%u] found in %i:%i:%i savestate\n",cType,c->x,c->y,c->z);
			goto chungusLoadEnd;
		}
		if(b >= end){break;}
	}

	chungusLoadEnd:
	fclose(fp);
}

#include <stdio.h>
void chungusSave(chungus *c){
	#ifdef __EMSCRIPTEN__
	return;
	#endif
	if(c == NULL){ return; }
	PROFILE_START();
	if(saveLoadBuffer == NULL)  { saveLoadBuffer   = malloc(4100*4096); }
	if(compressedBuffer == NULL){ compressedBuffer = malloc(4100*4096); }

	u8 *cbuf = saveLoadBuffer;
	for(int x=0;x<16;x++){
	for(int y=0;y<16;y++){
	for(int z=0;z<16;z++){
		if(c->chunks[x][y][z] == NULL){continue;}
		cbuf = chunkSave(c->chunks[x][y][z],cbuf);
	}
	}
	}
	cbuf = itemDropSaveChungus (c,cbuf);
	cbuf = animalSaveChungus   (c,cbuf);
	cbuf = fireSaveChungus     (c,cbuf);
	cbuf = throwableSaveChungus(c,cbuf);

	size_t len = LZ4_compress_default((const char *)saveLoadBuffer, (char *)compressedBuffer, cbuf - saveLoadBuffer, 4100*4096);
	if(len == 0){
		fprintf(stderr,"No Data for chungus %i:%i:%i\n",c->x,c->y,c->z);
		PROFILE_STOP();
		return;
	}
	size_t written = 0;
	FILE *fp = fopen(chungusGetFilename(c),"wb");
	if(fp == NULL){
		fprintf(stderr,"Error opening %s for writing\n",chungusGetFilename(c));
		PROFILE_STOP();
		return;
	}
	for(int i=0;i<64;i++){
		written += fwrite(compressedBuffer+written,1,len-written,fp);
		if(written >= len){
			fclose(fp);
			c->clientsUpdated |= ((u64)1 << 31);
			PROFILE_STOP();
			return;
		}
	}
	fclose(fp);
	fprintf(stderr,"Write error on chungus %i:%i:%i\n",c->x,c->y,c->z);
	PROFILE_STOP();
}

static void savegameParseLine(const char *line){
	int argc;
	char **argv;

	argv = splitArgs(line,&argc);
	if(argc == 0)          {return;}
	if(argv[0][0] == 0)    {return;}
	if(isspace((u8)argv[0][0])){return;}

	if(strcmp(argv[0],"WorldSeed") == 0){
		if(argc < 2){return;}
		optionWorldSeed = atoi(argv[1]);
		return;
	}

	if(strcmp(argv[0],"Time") == 0){
		if(argc < 2){return;}
		gtimeSetTime(atoi(argv[1]));
		return;
	}

	if(strcmp(argv[0],"Rain") == 0){
		if(argc < 2){return;}
		rainIntensity = atoi(argv[1]);
		return;
	}

	if(strcmp(argv[0],"CloudDensity") == 0){
		if(argc < 3){return;}
		cloudDensityMin  = atoi(argv[1]);
		cloudGDensityMin = atoi(argv[2]);
		return;
	}

	if(strcmp(argv[0],"CloudOffset") == 0){
		if(argc < 4){return;}
		cloudOff = vecNew(atof(argv[1]),atof(argv[2]),atof(argv[3]));
		return;
	}

	if(strcmp(argv[0],"WindVel") == 0){
		if(argc < 4){return;}
		windVel = vecNew(atof(argv[1]),atof(argv[2]),atof(argv[3]));
		return;
	}

	if(strcmp(argv[0],"WindGVel") == 0){
		if(argc < 4){return;}
		windGVel = vecNew(atof(argv[1]),atof(argv[2]),atof(argv[3]));
		return;
	}
}

void savegameLoad(){
	size_t len = 0;
	char *b,*line;
	#ifdef __EMSCRIPTEN__
	return;
	#endif
	checkValidSavegame(optionSavegame);
	b = loadFile(savegameFileName(optionSavegame),&len);
	if((b == NULL) || (len == 0)){return;}

	line = b;
	for(uint i=0;i<len;i++){
		if(b[i] == '\r'){b[i] = 0;}
		if(b[i] == '\n'){
			b[i] = 0;
			savegameParseLine(line);
			line = &b[i+1];
		}
	}
	savegameParseLine(line);
}

void savegameSave(){
	static char buf[512];
	char *b;

	b  = buf;
	b += snprintf(b,sizeof(buf)-(b-buf),"SaveFormat 1\n");
	b += snprintf(b,sizeof(buf)-(b-buf),"WorldSeed %i\n",optionWorldSeed);
	b += snprintf(b,sizeof(buf)-(b-buf),"Time %u\n",gtimeGetTime());
	b += snprintf(b,sizeof(buf)-(b-buf),"CloudDensity %u %u\n",cloudDensityMin,cloudGDensityMin);
	b += snprintf(b,sizeof(buf)-(b-buf),"Rain %u\n",rainIntensity);
	b += snprintf(b,sizeof(buf)-(b-buf),"CloudOffset %f %f %f\n",cloudOff.x,cloudOff.y,cloudOff.z);
	b += snprintf(b,sizeof(buf)-(b-buf),"WindVel %f %f %f\n",windVel.x,windVel.y,windVel.z);
	b += snprintf(b,sizeof(buf)-(b-buf),"WindGVel %f %f %f\n",windGVel.x,windGVel.y,windGVel.z);

	saveFile(savegameFileName(optionSavegame),buf,strlen(buf));
}

void playerSafeSave(){
	static uint pi=0;
	static uint lastSave=0;
	const uint cm = getTicks();
	if(cm < lastSave+2048){return;}
	lastSave = cm;
	if(++pi >= clientCount){pi=0;}
	if(clients[pi].state){return;}
	characterSaveData(clients[pi].c,clients[pi].playerName);
}
