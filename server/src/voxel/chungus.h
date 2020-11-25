#pragma once

#include "../../../common/src/common.h"

#define CHUNGUS_SIZE (16*16)

struct chungus {
	u8     x,y,z;
	ivec   spawn;
	u64    clientsSubscribed;
	u64    clientsUpdated;
	u64    freeTimer;
	void  *nextFree;
	chunk *chunks[16][16][16];
};

extern chungus chungusList[1 << 12];
extern uint chungusCount;
extern u64  freeTime;
extern int loadShed;

void         chungusInit             ();
chungus     *chungusNew              (u8 x,u8 y, u8 z);
void         chungusWorldGenLoad     (chungus *c);
void         chungusFree             (chungus *c);
void         chungusBox              (chungus *c, int x, int y, int z, int w, int h, int d, u8 block);
void         chungusBoxIfEmpty       (chungus *c, int x, int y, int z, int w, int h, int d, u8 block);
void         chungusBoxF             (chungus *c, int x, int y, int z, int w, int h, int d, u8 block);
void         chungusBoxFWG           (chungus *c, int x,int y,int z,int w,int h,int d);
void         chungusBoxSphere        (chungus *c, int x, int y, int z, int r, u8 block);
void         chungusFill             (chungus *c, int x, int y, int z, u8 block);
void         chungusSetB             (chungus *c, int x, int y, int z, u8 block);
u8           chungusGetB             (chungus *c, int x, int y, int z);
chunk       *chungusGetChunk         (chungus *c, int x, int y, int z);
void         chungusSetSpawn         (chungus *c, const ivec spawn);
void         chungusSubscribePlayer  (chungus *c, uint p);
int          chungusUnsubscribePlayer(chungus *c, uint p);
uint         chungusIsSubscribed     (chungus *c, uint p);
int          chungusUpdateClient     (chungus *c, uint p);
uint         chungusIsUpdated        (chungus *c, uint p);
void         chungusSetUpdated       (chungus *c, uint p);
void         chungusUnsetUpdated     (chungus *c, uint p);
void         chungusSetAllUpdated    (chungus *c, u64 nUpdated);
int          chungusGetHighestP      (chungus *c, int x, int *retY, int z);
float        chungusDistance         (const character *cam, const chungus *chng);
uint         chungusFreeOldChungi    (u64 threshold);
void         chungusUnsubFarChungi   ();

inline uint chungusGetActiveCount(){
	return chungusCount;
}
inline void chungusSetActiveCount(uint i){
	chungusCount = i;
}
inline chungus *chungusGetActive(uint i){
	if(i >= chungusCount){return NULL;}
	return &chungusList[i];
}
