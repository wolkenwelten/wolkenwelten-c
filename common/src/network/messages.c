#include "messages.h"

#include "../common.h"

#include <string.h>

packet packetBuffer;

void msgRequestPlayerSpawnPos(){
	packet *p = &packetBuffer;
	packetQueueToServer(p,1,0);
}

void msgPlayerSetPos(uint c, const vec pos, const vec rot){
	packet *p = &packetBuffer;
	p->val.f[0] = pos.x;
	p->val.f[1] = pos.y;
	p->val.f[2] = pos.z;
	p->val.f[3] = rot.yaw;
	p->val.f[4] = rot.pitch;
	p->val.f[5] = rot.roll;
	packetQueue(p,1,6*4,c);
}

void msgRequestChungus(uint x, uint y, uint z){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	packetQueueToServer(p,2,3*4);
}

void msgDirtyChunk(uint x, uint y, uint z){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	packetQueueToServer(p,31,3*4);
}

void msgPlaceBlock(uint x, uint y, uint z, u8 b){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	p->val.i[3] = b;
	packetQueueToServer(p,3,4*4);
}

void msgMineBlock(uint x, uint y, uint z, u8 b){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	p->val.i[3] = b;
	packetQueue(p,4,4*4,-1);
}

void msgGoodbye(){
	packet *p = &packetBuffer;
	packetQueueToServer(p,5,0);
}

void msgBlockMiningUpdate(uint c, u16 x, u16 y, u16 z, u16 damage, int count, int i){
	packet *p   = &packetBuffer;
	p->val.i[0] = (x & 0xFFFF) | ((y      & 0xFFFF)<<16) ;
	p->val.i[1] = (z & 0xFFFF) | ((damage & 0xFFFF)<<16) ;
	p->val.i[2] = count;
	p->val.i[3] = i;
	packetQueue(p,6,4*4,c);
}

void msgSendChungusComplete(uint c, int x, int y, int z){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	packetQueue(p,7,3*4,c);
}

void msgCharacterGotHit(uint c,int pwr){
	packet *p = &packetBuffer;
	p->val.i[0] = pwr;
	p->val.i[1] = c;
	packetQueueExcept(p,8,2*4,c);
}

void msgItemDropNew(uint c, const vec pos, const vec vel, const item *itm){
	packet *p = &packetBuffer;
	p->val.f[0] = pos.x;
	p->val.f[1] = pos.y;
	p->val.f[2] = pos.z;
	p->val.f[3] = vel.x;
	p->val.f[4] = vel.y;
	p->val.f[5] = vel.z;
	p->val.i[6] = itm->ID;
	p->val.i[7] = itm->amount;
	packetQueue(p,10,8*4,c);
}

void msgNewGrenade(const vec pos, const vec rot, float pwr){
	packet *p = &packetBuffer;
	p->val.f[0] = pos.x;
	p->val.f[1] = pos.y;
	p->val.f[2] = pos.z;
	p->val.f[3] = rot.yaw;
	p->val.f[4] = rot.pitch;
	p->val.f[5] = rot.roll;
	p->val.f[6] = pwr;
	packetQueueToServer(p,11,7*4);
}

void msgBeamBlast(const vec pos, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft){
	packet *p = &packetBuffer;
	p->val.f[0] = pos.x;
	p->val.f[1] = pos.y;
	p->val.f[2] = pos.z;
	p->val.f[3] = rot.yaw;
	p->val.f[4] = rot.pitch;
	p->val.f[5] = beamSize;
	p->val.f[6] = damageMultiplier;
	p->val.f[7] = recoilMultiplier;
	p->val.i[8] = hitsLeft;
	packetQueueToServer(p,12,9*4);
}

void msgPlayerMove(uint c, const vec dpos, const vec drot){
	packet *p = &packetBuffer;
	p->val.f[0] = dpos.x;
	p->val.f[1] = dpos.y;
	p->val.f[2] = dpos.z;
	p->val.f[3] = drot.yaw;
	p->val.f[4] = drot.pitch;
	p->val.f[5] = drot.roll;
	p->val.i[6] = c;
	packetQueue(p,13,7*4,c);
}

void msgPlayerName(uint c, uint i, const char *name){
	packet *p = &packetBuffer;
	p->val.s[0] = i;
	strncpy((char *)&p->val.c[2],name,31);
	p->val.c[33] = 0;
	packetQueue(p,14,36,c);
}

// 15 = parsePlayerPos ???

// 16 = parseChatMsg ???

// 17 = parseDyingMsg ???

// 18 = chunkData ???

void msgSetPlayerCount(uint playerLeaving, uint playerMax){
	packet *p = &packetBuffer;
	p->val.u[0] = playerMax;
	p->val.u[1] = playerLeaving;
	packetQueue(p,19,2*4,-1);
}

void msgPickupItem(uint c, u16 ID, u16 amount){
	packet *p = &packetBuffer;
	p->val.s[0] = ID;
	p->val.s[1] = amount;
	packetQueue(p,20,2*2,c);
}

void msgGrenadeExplode(const vec pos,float pwr, int style){
	packet *p = &packetBuffer;
	p->val.f[0] = pos.x;
	p->val.f[1] = pos.y;
	p->val.f[2] = pos.z;
	p->val.f[3] = pwr;
	p->val.i[4] = style;
	packetQueue(p,22,5*4,-1);
}

void msgGrenadeUpdate(uint c, const vec pos, const vec vel, int count, int i){
	packet *p = &packetBuffer;
	p->val.f[0] = pos.x;
	p->val.f[1] = pos.y;
	p->val.f[2] = pos.z;
	p->val.f[3] = vel.x;
	p->val.f[4] = vel.y;
	p->val.f[5] = vel.z;
	p->val.i[6] = count;
	p->val.i[7] = i;
	packetQueue(p,23,8*4,c);
}

void msgFxBeamBlaster(uint c, const vec pa, const vec pb, float beamSize, float damageMultiplier, float recoilMultiplier){
	packet *p = &packetBuffer;
	p->val.f[0] = pa.x;
	p->val.f[1] = pa.y;
	p->val.f[2] = pa.z;
	p->val.f[3] = pb.x;
	p->val.f[4] = pb.y;
	p->val.f[5] = pb.z;
	p->val.f[6] = beamSize;
	p->val.f[7] = damageMultiplier;
	p->val.f[8] = recoilMultiplier;

	packetQueueExcept(p,24,9*4,c);
}

void msgItemDropUpdate(uint c, const vec pos, const vec vel, u16 i, u16 len, u16 itemID, u16 amount){
	packet *p = &packetBuffer;

	p->val.s[0] = i;
	p->val.s[1] = len;

	p->val.s[2] = itemID;
	p->val.s[3] = amount;

	p->val.f[2] = pos.x;
	p->val.f[3] = pos.y;
	p->val.f[4] = pos.z;
	p->val.f[5] = vel.x;
	p->val.f[6] = vel.y;
	p->val.f[7] = vel.z;

	packetQueue(p,25,8*4,c);
}

void msgPlayerDamage(uint c, i16 hp, u16 target, u16 cause, u16 culprit){
	packet *p = &packetBuffer;
	p->val.s[0] = hp;
	p->val.s[1] = target;
	p->val.s[2] = cause;
	p->val.s[3] = culprit;
	packetQueue(p,26,4*2,c);
}

void msgUnsubChungus(uint x, uint y, uint z){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	packetQueueToServer(p,27,3*4);
}

void msgPlayerSetData(uint c, int hp, int activeItem, u32 flags){
	packet *p = &packetBuffer;
	p->val.i[0] = hp;
	p->val.i[1] = activeItem;
	p->val.u[2] = flags;
	packetQueue(p,28,3*4,c);
}

void msgPlayerSetInventory(uint c,const item *itm, size_t itemCount){
	packet *p = &packetBuffer;
	for(uint i=0;i<itemCount;i++){
		p->val.s[(i<<1)  ] = itm[i].ID;
		p->val.s[(i<<1)+1] = itm[i].amount;
	}
	packetQueue(p,29,itemCount*4,c);
}
