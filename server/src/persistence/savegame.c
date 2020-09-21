#include "savegame.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/itemDrop.h"
#include "../misc/options.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/misc/lz4.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

u8 *saveLoadBuffer   = NULL;
u8 *compressedBuffer = NULL;

void bigchungusSafeSaveClient(const bigchungus *c, const character *chara){
	if(chara == NULL){return;}
	ivec cp = ivecShrS(ivecNewV(chara->pos),8);
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

void bigchungusSafeSave(const bigchungus *c){
	static u64 lastSave = 0;
	if(getMillis() < lastSave+1000){return;}
	lastSave = getMillis();

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

static const char *chungusGetFilename(chungus *c){
	static char buf[64];
	snprintf(buf,sizeof(buf)-1,"save/%s/%02X%02X%02X.chunk",optionSavegame,(c->x >> 8)&0xFF,(c->y >> 8)&0xFF,(c->z >> 8)&0xFF);
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
		if(itemDrops[i].ent->curChungus != c){continue;}
		buf = itemDropSave(&itemDrops[i],buf);
	}
	return buf;
}

static void *animalSave(const animal *e, void *buf){
	u8    *b = (u8    *)buf;
	float *f = (float *)buf;

	b[ 0] = 0x03;
	b[ 1] = e->flags;
	b[ 2] = e->type;
	b[ 3] = e->state;

	b[ 4] = e->health;
	b[ 5] = e->hunger;
	b[ 6] = e->thirst;
	b[ 7] = e->sleepy;

	b[ 8] = e->age;
	b[ 9] = 0;
	b[10] = 0;
	b[11] = 0;

	f[ 3] = e->pos.x;
	f[ 4] = e->pos.y;
	f[ 5] = e->pos.z;
	f[ 6] = e->rot.yaw;
	f[ 7] = e->rot.pitch;
	f[ 8] = e->vel.x;
	f[ 9] = e->vel.y;
	f[10] = e->vel.z;

	return b+11*4;
}

static const void *animalLoad(const void *buf){
	u8 *b     = (u8 *)buf;
	float *f  = (float   *)buf;
	animal *e = animalNew(vecNewP(&f[3]),b[2]);
	if(e == NULL){return b+12*4;}

	e->rot    = vecNewP(&f[6]);
	e->rot.roll = 0.f;
	e->vel    = vecNewP(&f[8]);

	e->flags  = b[ 1];
	e->state  = b[ 3];

	e->health = b[ 4];
	e->hunger = b[ 5];
	e->thirst = b[ 6];
	e->sleepy = b[ 7];

	e->age    = b[ 8];

	return b+12*4;
}

static void *animalSaveChungus(const chungus *c,void *b){
	if(c == NULL){return b;}
	for(uint i=0;i<animalCount;i++){
		if(animalList[i].curChungus != c){continue;}
		b = animalSave(&animalList[i],b);
	}
	return b;
}

void chungusLoad(chungus *c){
	if(c == NULL)               {return;}
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
			default:
				fprintf(stderr,"Unknown id found in %i:%i:%i savestate\n",c->x>>8,c->y>>8,c->z>>8);
				goto chungusLoadEnd;
		}
		if(b >= end){break;}
	}

	chungusLoadEnd:
	fclose(fp);
}

void chungusSave(chungus *c){
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
