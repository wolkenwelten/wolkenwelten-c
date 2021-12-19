#pragma once

#include "../voxel/chunk.h"
#include "../../../common/src/common.h"

#define CHUNGUS_COORDS (16)
#define CHUNGUS_SIZE (CHUNGUS_COORDS*CHUNK_SIZE)

struct chungus {
	u8     x,y,z;
	u8     sx,sy,sz;
	u64    clientsSubscribed;
	u64    clientsUpdated;
	u64    freeTimer;
	void  *nextFree;
	beingList bl;
	chunk *chunks[CHUNGUS_COORDS][CHUNGUS_COORDS][CHUNGUS_COORDS];
};

extern chungus *chungusList;
extern uint chungusCount,chungusFreeCount;
extern u64  freeTime;
extern int loadShed;

void         chungusInit             ();
chungus     *chungusNew              (u8 x,u8 y, u8 z);
void         chungusWorldGenLoad     (chungus *c);
void         chungusFree             (chungus *c);
void         chungusBox              (chungus *c, int x, int y, int z, int w, int h, int d, blockId block);
void         chungusBoxIfEmpty       (chungus *c, int x, int y, int z, int w, int h, int d, blockId block);
void         chungusBoxF             (chungus *c, int x, int y, int z, int w, int h, int d, blockId block);
void         chungusBoxFWG           (chungus *c, int x, int y, int z, int w, int h, int d);
void         chungusBoxSphere        (chungus *c, int x, int y, int z, int r, blockId block);
void         chungusFill             (chungus *c, int x, int y, int z, blockId block);
void         chungusSetB             (chungus *c, int x, int y, int z, blockId block);
blockId      chungusGetB             (chungus *c, int x, int y, int z);
void         chungusSetFluid         (chungus *c, int x, int y, int z, int level);
u8           chungusGetFluid         (chungus *c, int x, int y, int z);
vec          chungusGetPos           (const chungus *c);
chunk       *chungusGetChunk         (chungus *c, int x, int y, int z);
void         chungusSetSpawn         (chungus *c, const vec spawn);
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
