#include "messages.h"

#include "../main.h"
#include "../game/entity.h"
#include "../game/character.h"
#include "../game/grapplingHook.h"
#include "../network/packet.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"

#include <string.h>
#include <time.h>

void msgSendPlayerPos(){
	packetLarge p;
	item *itm = characterGetItemBarSlot(player,player->activeItem);

	p.val.f[ 0] = player->x;
	p.val.f[ 1] = player->y;
	p.val.f[ 2] = player->z;
	p.val.f[ 3] = player->yaw;
	p.val.f[ 4] = player->pitch;
	p.val.f[ 5] = player->roll;
	p.val.f[ 6] = player->vx;
	p.val.f[ 7] = player->vy;
	p.val.f[ 8] = player->vz;
	p.val.f[ 9] = player->yoff;

	if(player->hook != NULL){
		p.val.i[10] = 1;
		p.val.f[11] = player->hook->ent->x;
		p.val.f[12] = player->hook->ent->y;
		p.val.f[13] = player->hook->ent->z;
	} else {
		p.val.i[10] = 0;
	}
	p.val.i[14] = player->blockMiningX;
	p.val.i[15] = player->blockMiningY;
	p.val.i[16] = player->blockMiningZ;
	p.val.i[17] = itm->ID;

	packetQueueL(&p,1);
}

void msgBeamBlast(int ID, float x, float y, float z, float yaw, float pitch, float roll, float pwr){
	packetMedium p;
	p.target = ID;
	p.val.f[0] = x;
	p.val.f[1] = y;
	p.val.f[2] = z;
	p.val.f[3] = yaw;
	p.val.f[4] = pitch;
	p.val.f[5] = roll;
	p.val.i[6] = pwr;
	packetQueueM(&p,4);
}

void msgNewGrenade(int ID, float x, float y, float z, float yaw, float pitch, float roll, float pwr){
	packetMedium p;
	p.target = ID;
	p.val.f[0] = x;
	p.val.f[1] = y;
	p.val.f[2] = z;
	p.val.f[3] = yaw;
	p.val.f[4] = pitch;
	p.val.f[5] = roll;
	p.val.i[6] = pwr;
	packetQueueM(&p,3);
}

void msgItemDropNew(float x, float y, float z, float vx, float vy, float vz, int ID, int amount){
	packetMedium p;
	p.target = amount;
	p.val.f[0] = x;
	p.val.f[1] = y;
	p.val.f[2] = z;
	p.val.f[3] = vx;
	p.val.f[4] = vy;
	p.val.f[5] = vz;
	p.val.i[6] = ID;
	packetQueueM(&p,2);
}

void msgPlaceBlock(int x, int y, int z, uint8_t b){
	packetSmall p;
	p.target = b;
	p.val.i[0] = x;
	p.val.i[1] = y;
	p.val.i[2] = z;
	packetQueueS(&p,3);
}

void msgMineBlock(int x, int y, int z, uint8_t b){
	packetSmall p;
	p.target = b;
	p.val.i[0] = x;
	p.val.i[1] = y;
	p.val.i[2] = z;
	packetQueueS(&p,4);
}

void msgRequestPlayerSpawnPos(){
	packetSmall p;
	packetQueueS(&p,1);
}

void msgGoodbye(){
	packetSmall p;
	packetQueueS(&p,5);
}

void msgRequestChungus(int x, int y, int z){
	packetSmall p;
	p.val.i[0] = x;
	p.val.i[1] = y;
	p.val.i[2] = z;
	packetQueueS(&p,2);
}

void msgParseGetChunk(packetHuge *p){
	int x = p->val.i[1024];
	int y = p->val.i[1025];
	int z = p->val.i[1026];
	chungus *chng =  worldGetChungus(x>>8,y>>8,z>>8);
	chunk *chnk = chungusGetChunkOrNew(chng,x,y,z);
	if(chnk == NULL){return;}
	memcpy(chnk->data,p->val.c,sizeof(chnk->data));
	chnk->ready &= ~15;
}
