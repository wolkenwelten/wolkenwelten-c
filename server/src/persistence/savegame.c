#include "savegame.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/entity.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/time.h"
#include "../game/water.h"
#include "../misc/options.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/game/weather.h"
#include "../../../common/src/network/messages.h"
#include "../../../common/src/misc/lz4.h"
#include "../../../common/src/misc/misc.h"

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
	snprintf(buf,sizeof(buf)-1,"save/%s/%02X%02X%02X.chunk",optionSavegame,c->x,c->y,c->z);
	buf[sizeof(buf)-1] = 0;
	return buf;
}

static u8 *chunkSave(chunk *c, u8 *buf){
	if((c->clientsUpdated & ((u64)1 << 31)) != 0){return buf;}
	buf[0] = 0x01;
	buf[1] = (c->x >> 4)&0xF;
	buf[2] = (c->y >> 4)&0xF;
	buf[3] = (c->z >> 4)&0xF;
	memcpy(buf+4,c->data,16*16*16);
	return buf+4100;
}

static const u8 *chunkLoad(chungus *c, const u8 *buf){
	if(buf[0] != 0x01){return buf;}

	int cx = buf[1] & 0xF;
	int cy = buf[2] & 0xF;
	int cz = buf[3] & 0xF;

	chunk *chnk = c->chunks[cx][cy][cz];
	if(chnk == NULL){
		c->chunks[cx][cy][cz] = chnk = chunkNew(c->x+(cx<<4),c->y+(cy<<4),c->z+(cz<<4));
	}
	chnk->clientsUpdated = 0;
	memcpy(chnk->data,&buf[4],4096);

	return buf+4100;
}

static void *itemDropSave(const itemDrop *i, void *buf){
	u8    *b = (u8 *)    buf;
	u16   *s = (u16 *)   buf;
	float *f = (float *) buf;

	if(i      == NULL){return b;}
	if(i->ent == NULL){return b;}

	b[0] = 0x02;
	b[1] = 0;

	s[1] = i->itm.ID;
	s[2] = i->itm.amount;
	s[3] = 0;

	f[2] = i->ent->pos.x;
	f[3] = i->ent->pos.y;
	f[4] = i->ent->pos.z;
	f[5] = i->ent->vel.x;
	f[6] = i->ent->vel.y;
	f[7] = i->ent->vel.z;

	return b+32;
}

static const void *itemDropLoad(const void *buf){
	u8    *b = (u8 *)    buf;
	u16   *s = (u16 *)   buf;
	float *f = (float *) buf;

	itemDrop *id = itemDropNew();
	if(id == NULL){return b+32;}
	id->itm.ID     = s[1];
	id->itm.amount = s[2];

	id->ent = entityNew(vecNewP(&f[2]),vecZero());
	if(id->ent == NULL){return b+32;}
	id->ent->vel = vecNewP(&f[5]);

	return b+32;
}

static void *itemDropSaveChungus(const chungus *c,void *buf){
	if(c == NULL){return buf;}
	for(uint i=0;i<itemDropCount;i++){
		if(itemIsEmpty(&itemDropList[i].itm))   {continue;}
		if(itemDropList[i].ent == NULL)         {continue;}
		if(itemDropList[i].ent->curChungus != c){continue;}
		buf = itemDropSave(&itemDropList[i],buf);
	}
	return buf;
}

static void *fireSave(const fire *f, void *buf){
	u8    *b = (u8  *)buf;
	u16   *u = (u16 *)buf;
	i16   *s = (i16 *)buf;

	if(f == NULL){return b;}

	b[0] = 0x04;
	b[1] = 0;

	u[1] = f->x;
	u[2] = f->y;
	u[3] = f->z;

	s[4] = f->strength;
	s[5] = f->blockDmg;
	s[6] = f->oxygen;

	return b+14;
}

static const void *fireLoad(const void *buf){
	u8    *b = (u8  *)buf;
	u16   *u = (u16 *)buf;
	i16   *s = (i16 *)buf;

	fireNewF(u[1],u[2],u[3],s[4],s[5],s[6]);
	return b+14;
}

static void *fireSaveChungus(const chungus *c,void *buf){
	if(c == NULL){return buf;}
	for(uint i=0;i<fireCount;i++){
		if(c->x != (fireList[i].x >> 8)){continue;}
		if(c->y != (fireList[i].y >> 8)){continue;}
		if(c->z != (fireList[i].z >> 8)){continue;}
		buf = fireSave(&fireList[i],buf);
	}
	return buf;
}

static void *waterSave(const water *w, void *buf){
	u8    *b = (u8  *)buf;
	u16   *u = (u16 *)buf;
	i16   *s = (i16 *)buf;

	if(w == NULL){return b;}

	b[0] = 0x05;
	b[1] = 0;

	u[1] = w->x;
	u[2] = w->y;
	u[3] = w->z;

	s[4] = w->amount;

	return b+10;
}

static const void *waterLoad(const void *buf){
	u8    *b = (u8  *)buf;
	u16   *u = (u16 *)buf;
	i16   *s = (i16 *)buf;

	waterNewF(u[1],u[2],u[3],s[4]);
	return b+10;
}

static void *waterSaveChungus(const chungus *c,void *buf){
	if(c == NULL){return buf;}
	for(uint i=0;i<waterCount;i++){
		if(c->x != (waterList[i].x >> 8)){continue;}
		if(c->y != (waterList[i].y >> 8)){continue;}
		if(c->z != (waterList[i].z >> 8)){continue;}
		buf = waterSave(&waterList[i],buf);
	}
	return buf;
}

static void *animalSave(const animal *e, void *buf){
	u8    *b = (u8    *)buf;
	u32   *u = (u32   *)buf;
	float *f = (float *)buf;
	if(e->type == 0){return b;}

	b[ 0] = 0x03;
	b[ 1] = e->flags;
	b[ 2] = e->type;
	b[ 3] = e->state;

	b[ 4] = e->health;
	b[ 5] = e->hunger;
	b[ 6] = e->pregnancy;
	b[ 7] = e->sleepy;

	b[ 8] = e->age;
	b[ 9] = 0;
	b[10] = 0;
	b[11] = 0;

	f[ 3] = e->pos.x;
	f[ 4] = e->pos.y;
	f[ 5] = e->pos.z;

	f[ 6] = e->vel.x;
	f[ 7] = e->vel.y;
	f[ 8] = e->vel.z;

	f[ 9] = e->rot.yaw;
	f[10] = e->rot.pitch;
	u[11] = e->stateTicks;

	return b+12*4;
}

static const void *animalLoad(const void *buf){
	u8 *b         = (u8 *)buf;
	u32 *u        = (u32 *)buf;
	float *f      = (float *)buf;
	if(b[ 2] == 0){return b+12*4;}
	animal *e     = animalNew(vecNewP(&f[3]),b[2],0);
	if(e == NULL){return b+12*4;}

	e->flags      = b[ 1];
	e->type       = b[ 2];
	e->state      = b[ 3];

	e->health     = b[ 4];
	e->hunger     = b[ 5];
	e->pregnancy  = b[ 6];
	e->sleepy     = b[ 7];

	e->age        = b[ 8];

	e->pos        = vecNewP(&f[3]);
	e->vel        = vecNewP(&f[6]);

	e->rot.yaw    = f[ 9];
	e->rot.pitch  = f[10];
	e->rot.roll   = 0.f;

	e->stateTicks = u[11];
	e->target     = 0;

	return b+12*4;
}

static void *animalSaveChungus(const chungus *c,void *b){
	if(c == NULL){return b;}
	const u32 cc = c->x | (c->y << 8) | (c->z << 16);
	for(uint i=0;i<animalCount;i++){
		const vec *p = &animalList[i].pos;
		const u32 ac = ((uint)p->x >> 8) | ((uint)p->y & 0xFF00) | (((uint)p->z << 8) & 0xFF0000);
		if(ac != cc){continue;}
		b = animalSave(&animalList[i],b);
	}
	return b;
}

static void characterParseDataLine(character *p, const char *line){
	int argc;
	char **argv;

	argv = splitArgs(line,&argc);
	if(argc == 0)          {return;}
	if(argv[0][0] == 0)    {return;}
	if(isspace(argv[0][0])){return;}

	if(strcmp(argv[0],"Position") == 0){
		if(argc < 7){return;}
		p->pos = vecNew(atof(argv[1]),atof(argv[2]),atof(argv[3]));
		p->rot = vecNew(atof(argv[4]),atof(argv[5]),atof(argv[6]));
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

static const char *savegameFileName(const char *name){
	static char filename[128];
	snprintf(filename,sizeof(filename)-1,"save/%s/world.global",name);
	filename[sizeof(filename)-1] = 0;
	return filename;
}

static void checkValidSavegame(const char *name){
	static char buf[128];
	if(!isDir("save")){makeDir("save");}
	snprintf(buf,sizeof(buf)-1,"save/%.63s",name);
	buf[sizeof(buf)-1] = 0;
	if(!isDir(buf)){makeDir(buf);}
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

void bigchungusSafeSave(const bigchungus *c){
	static u64 lastSave = 0;
	if(getTicks() < lastSave+1000){return;}
	lastSave = getTicks();

	for(int x=127;x <= 129;x++){
		for(int y=1;y <= 3;y++){
			for(int z=127;z <= 129;z++){
				if(c->chungi[x][y][z] == NULL){continue;}
				chungusSave(c->chungi[x][y][z]);
			}
		}
	}

	for(uint i=0;i<clientCount;i++){
		if(clients[i].state == 2){ continue; }
		if(clients[i].c == NULL ){ continue; }
		bigchungusSafeSaveClient(c,clients[i].c);
	}
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
		u8 id = *b;
		switch(id){
		case 1:
			b = chunkLoad(c,b);
			break;
		case 2:
			b = itemDropLoad(b);
			break;
		case 3:
			b = animalLoad(b);
			break;
		case 4:
			b = fireLoad(b);
			break;
		case 5:
			b = waterLoad(b);
			break;
		default:
			fprintf(stderr,"Unknown id[%u] found in %i:%i:%i savestate\n",id,c->x,c->y,c->z);
			goto chungusLoadEnd;
		}
		if(b >= end){break;}
	}

	chungusLoadEnd:
	fclose(fp);
}

void chungusSave(chungus *c){
	#ifdef __EMSCRIPTEN__
	return;
	#endif
	if(c == NULL)                                 { return; }
	if((c->clientsUpdated & ((u64)1 << 31)) != 0) { return; }
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
	cbuf = itemDropSaveChungus(c,cbuf);
	cbuf = animalSaveChungus  (c,cbuf);
	cbuf = fireSaveChungus    (c,cbuf);
	cbuf = waterSaveChungus   (c,cbuf);

	size_t len = LZ4_compress_default((const char *)saveLoadBuffer, (char *)compressedBuffer, cbuf - saveLoadBuffer, 4100*4096);
	if(len == 0){
		fprintf(stderr,"No Data for chungus %i:%i:%i\n",c->x,c->y,c->z);
		return;
	}
	size_t written = 0;
	FILE *fp = fopen(chungusGetFilename(c),"wb");
	if(fp == NULL){
		fprintf(stderr,"Error opening %s for writing\n",chungusGetFilename(c));
		return;
	}
	for(int i=0;i<64;i++){
		written += fwrite(compressedBuffer+written,1,len-written,fp);
		if(written >= len){
			fclose(fp);
			c->clientsUpdated |= ((u64)1 << 31);
			return;
		}
	}
	fclose(fp);
	fprintf(stderr,"Write error on chungus %i:%i:%i\n",c->x,c->y,c->z);
}

static void savegameParseLine(const char *line){
	int argc;
	char **argv;

	argv = splitArgs(line,&argc);
	if(argc == 0)          {return;}
	if(argv[0][0] == 0)    {return;}
	if(isspace(argv[0][0])){return;}

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
	b += snprintf(b,sizeof(buf)-(b-buf+1),"SaveFormat 1\n");
	b += snprintf(b,sizeof(buf)-(b-buf+1),"WorldSeed %i\n",optionWorldSeed);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"Time %u\n",gtimeGetTime());
	b += snprintf(b,sizeof(buf)-(b-buf+1),"CloudDensity %u %u\n",cloudDensityMin,cloudGDensityMin);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"Rain %u\n",rainIntensity);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"CloudOffset %f %f %f\n",cloudOff.x,cloudOff.y,cloudOff.z);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"WindVel %f %f %f\n",windVel.x,windVel.y,windVel.z);
	b += snprintf(b,sizeof(buf)-(b-buf+1),"WindGVel %f %f %f\n",windGVel.x,windGVel.y,windGVel.z);

	buf[sizeof(buf)-1] = 0;
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
