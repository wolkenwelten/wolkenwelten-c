#include "messages.h"

#include "../game/character.h"
#include "../game/blockMining.h"
#include "../game/grenade.h"
#include "../game/item.h"
#include "../game/itemDrop.h"
#include "../network/packet.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"

#include <string.h>

void msgPickupItem(int c, const item *i){
	packetSmall p;
	p.target = c;
	p.val.i[0] = i->ID;
	p.val.i[1] = i->amount;
	packetQueueS(&p,5,c);
}

void msgMineBlock(int x, int y, int z, uint8_t b){
	packetSmall p;
	p.target = b;
	p.val.i[0] = x;
	p.val.i[1] = y;
	p.val.i[2] = z;
	packetQueueS(&p,4,-1);
}

void msgGrenadeExplode(float x, float y, float z,float pwr, int style){
	packetMedium p;
	p.target = style;
	p.val.f[0] = x;
	p.val.f[1] = y;
	p.val.f[2] = z;
	p.val.f[3] = pwr;
	packetQueueM(&p,2,-1);
}

void msgCharacterGotHitBroadcast(int c,float pwr){
	packetSmall p;
	p.target = c;
	p.val.f[0] = pwr;
	packetQueueExceptS(&p,8,c);
}

void msgCharacterHit(int c, float x, float y, float z, float yaw, float pitch, float roll, float pwr){
	packetMedium p;
	p.target = c;
	p.val.f[0] = x;
	p.val.f[1] = y;
	p.val.f[2] = z;
	p.val.f[3] = yaw;
	p.val.f[4] = pitch;
	p.val.f[5] = roll;
	p.val.f[6] = pwr;
	packetQueueExceptM(&p,6,c);
}

void msgFxBeamBlaster(int c, float x1, float y1, float z1, float x2, float y2, float z2, float pwr){
	packetMedium p;
	p.target   = c;
	p.val.f[0] = x1;
	p.val.f[1] = y1;
	p.val.f[2] = z1;
	p.val.f[3] = x2;
	p.val.f[4] = y2;
	p.val.f[5] = z2;
	p.val.f[6] = pwr;
	packetQueueExceptM(&p,4,c);
	p.target = 65535;
	packetQueueM(&p,4,c);
}

void msgPlayerMove(int c, float dvx, float dvy, float dvz, float dyaw, float dpitch, float droll){
	packetMedium p;
	p.target   = c;
	p.val.f[0] = dvx;
	p.val.f[1] = dvy;
	p.val.f[2] = dvz;
	p.val.f[3] = dyaw;
	p.val.f[4] = dpitch;
	p.val.f[5] = droll;
	packetQueueM(&p,5, c);
}

void msgUpdatePlayer(int c){
	packetLarge rp;

	for(int i=0;i<clientCount;++i){
		if(i==c){continue;}
		if(clients[i].c == NULL){continue;}

		rp.val.f[ 0] = clients[i].c->x;
		rp.val.f[ 1] = clients[i].c->y;
		rp.val.f[ 2] = clients[i].c->z;
		rp.val.f[ 3] = clients[i].c->yaw;
		rp.val.f[ 4] = clients[i].c->pitch;
		rp.val.f[ 5] = clients[i].c->roll;
		rp.val.f[ 6] = clients[i].c->vx;
		rp.val.f[ 7] = clients[i].c->vy;
		rp.val.f[ 8] = clients[i].c->vz;
		rp.val.f[ 9] = clients[i].c->yoff;
		rp.val.i[10] = clients[i].c->hook;
		rp.val.f[11] = clients[i].c->hookx;
		rp.val.f[12] = clients[i].c->hooky;
		rp.val.f[13] = clients[i].c->hookz;
		rp.val.i[14] = clients[i].c->blockMiningX;
		rp.val.i[15] = clients[i].c->blockMiningY;
		rp.val.i[16] = clients[i].c->blockMiningZ;
		rp.val.i[17] = clients[i].c->activeItem;
		rp.val.i[18] = clients[i].c->hitOff;
		rp.target = i;
		packetQueueL(&rp,1,c);
	}

	itemDropUpdatePlayer(c);
	grenadeUpdatePlayer(c);
	blockMiningUpdatePlayer(c);
}

void msgRequestPlayerSpawnPos(int c, const packetSmall *pr){
	(void)pr;
	packetSmall p;
	int sx,sy,sz;
	worldGetSpawnPos(&sx,&sy,&sz);
	p.val.f[0] = ((float)sx)+0.5f;
	p.val.f[1] = ((float)sy)+2.0f;
	p.val.f[2] = ((float)sz)+0.5f;
	packetQueueS(&p,1,c);
}

void msgRequestChungus(int c, const packetSmall *p){
	addChungusToQueue(c,p->val.i[0],p->val.i[1],p->val.i[2]);
}

void msgSendChunk(int c, const chunk *chnk){
	packetHuge p;
	memcpy(p.val.c,chnk->data,sizeof(chnk->data));
	p.val.i[1024] = chnk->x;
	p.val.i[1025] = chnk->y;
	p.val.i[1026] = chnk->z;
	packetQueueH(&p,3,c);
}

void msgSendChungusComplete(int c, int x, int y, int z){
	packetSmall p;
	p.val.i[0] = x;
	p.val.i[1] = y;
	p.val.i[2] = z;
	packetQueueS(&p,7,c);
}
