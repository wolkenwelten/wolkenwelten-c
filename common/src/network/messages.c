#include "messages.h"

#include "../common.h"

#include <string.h>

packet packetBuffer;

void msgNOP(uint len){
	packet *p = &packetBuffer;
	memset(p->v.u8,0,len);
	packetQueueToServer(p,0,len);
}

void msgRequestPlayerSpawnPos(){
	packet *p = &packetBuffer;
	packetQueueToServer(p,1,0);
}

void msgPlayerSetPos(uint c, const vec pos, const vec rot){
	packet *p = &packetBuffer;

	p->v.f[0] = pos.x;
	p->v.f[1] = pos.y;
	p->v.f[2] = pos.z;

	p->v.f[3] = rot.yaw;
	p->v.f[4] = rot.pitch;
	p->v.f[5] = rot.roll;

	packetQueue(p,1,6*4,c);
}

void msgRequestChungus(u8 x, u8 y, u8 z){
	packet *p = &packetBuffer;

	p->v.u8[0] = x;
	p->v.u8[1] = y;
	p->v.u8[2] = z;
	p->v.u8[3] = 0;

	packetQueueToServer(p,2,4);
}

void msgUnsubChungus(u8 x, u8 y, u8 z){
	packet *p = &packetBuffer;

	p->v.u8[0] = x;
	p->v.u8[1] = y;
	p->v.u8[2] = z;
	p->v.u8[3] = 0;

	packetQueueToServer(p,27,4);
}

void msgDirtyChunk(u16 x, u16 y, u16 z){
	packet *p = &packetBuffer;
	p->v.u16[0] = x;
	p->v.u16[1] = y;
	p->v.u16[2] = z;
	p->v.u16[3] = 0;
	packetQueueToServer(p,31,2*4);
}

void msgPlaceBlock(u16 x, u16 y, u16 z, u8 b){
	packet *p = &packetBuffer;
	p->v.u16[0] = x;
	p->v.u16[1] = y;
	p->v.u16[2] = z;
	p->v.u16[3] = b;
	packetQueueToServer(p,3,2*4);
}

void msgMineBlock(u16 x, u16 y, u16 z, u8 b){
	packet *p = &packetBuffer;
	p->v.u16[0] = x;
	p->v.u16[1] = y;
	p->v.u16[2] = z;
	p->v.u16[3] = b;
	packetQueue(p,4,2*4,-1);
}

void msgGoodbye(){
	packet *p = &packetBuffer;
	packetQueueToServer(p,5,0);
}

void msgBlockMiningUpdate(uint c, u16 x, u16 y, u16 z, i16 damage, u16 count, u16 i){
	packet *p   = &packetBuffer;

	p->v.u16[0] = x;
	p->v.u16[1] = y;
	p->v.u16[2] = z;
	p->v.i16[3] = damage;
	p->v.u16[4] = i;
	p->v.u16[5] = count;

	packetQueue(p,6,6*2,c);
}

void msgSendChungusComplete(uint c, u8 x, u8 y, u8 z){
	packet *p = &packetBuffer;

	p->v.u8[0] = x;
	p->v.u8[1] = y;
	p->v.u8[2] = z;
	p->v.u8[3] = 0;

	packetQueue(p,7,4,c);
}

void msgItemDropNew(uint c, const vec pos, const vec vel, const item *itm){
	packet *p = &packetBuffer;

	p->v.f[0] = pos.x;
	p->v.f[1] = pos.y;
	p->v.f[2] = pos.z;

	p->v.f[3] = vel.x;
	p->v.f[4] = vel.y;
	p->v.f[5] = vel.z;

	p->v.i32[6] = itm->ID;
	p->v.i32[7] = itm->amount;

	packetQueue(p,10,8*4,c);
}

void msgNewGrenade(const vec pos, const vec rot, float pwr, int cluster, float clusterPwr){
	packet *p = &packetBuffer;

	p->v.f[0] = pos.x;
	p->v.f[1] = pos.y;
	p->v.f[2] = pos.z;

	p->v.f[3] = rot.yaw;
	p->v.f[4] = rot.pitch;
	p->v.f[5] = rot.roll;

	p->v.f[6] = pwr;
	p->v.i32[7] = cluster;
	p->v.f[8] = clusterPwr;

	packetQueueToServer(p,11,9*4);
}

void msgBeamBlast(const vec pos, const vec rot, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft){
	packet *p = &packetBuffer;

	p->v.f[0] = pos.x;
	p->v.f[1] = pos.y;
	p->v.f[2] = pos.z;
	p->v.f[3] = rot.yaw;
	p->v.f[4] = rot.pitch;
	p->v.f[5] = beamSize;
	p->v.f[6] = damageMultiplier;
	p->v.f[7] = recoilMultiplier;
	p->v.i32[8] = hitsLeft;

	packetQueueToServer(p,12,9*4);
}

void msgPlayerMove(uint c, const vec dpos, const vec drot){
	packet *p = &packetBuffer;

	p->v.f[0] = dpos.x;
	p->v.f[1] = dpos.y;
	p->v.f[2] = dpos.z;
	p->v.f[3] = drot.yaw;
	p->v.f[4] = drot.pitch;
	p->v.f[5] = drot.roll;
	p->v.i32[6] = c;

	packetQueue(p,13,7*4,c);
}

void msgPlayerName(uint c, u16 i, const char *name){
	packet *p = &packetBuffer;
	p->v.u16[0] = i;
	strncpy((char *)&p->v.u8[2],name,31);
	p->v.u8[33] = 0;
	packetQueue(p,14,36,c);
}

// 15 = parsePlayerPos ???

// 16 = parseChatMsg ???

// 17 = parseDyingMsg ???

// 18 = chunkData ???

void msgSetPlayerCount(u16 playerLeaving, u16 playerMax){
	packet *p = &packetBuffer;

	p->v.u16[0] = playerMax;
	p->v.u16[1] = playerLeaving;

	packetQueue(p,19,2*2,-1);
}

void msgPickupItem(uint c, u16 ID, i16 amount){
	packet *p = &packetBuffer;

	p->v.u16[0] = ID;
	p->v.i16[1] = amount;

	packetQueue(p,20,2*2,c);
}

void msgGrenadeExplode(const vec pos,float pwr, u16 style){
	packet *p = &packetBuffer;

	p->v.f[0] = pos.x;
	p->v.f[1] = pos.y;
	p->v.f[2] = pos.z;

	p->v.u16[6] = (u16)(pwr*256.f);
	p->v.u16[7] = style;

	packetQueue(p,22,4*4,-1);
}

void msgGrenadeUpdate(uint c, const vec pos, const vec vel, u16 i, u16 count){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = count;

	p->v.f[1]   = pos.x;
	p->v.f[2]   = pos.y;
	p->v.f[3]   = pos.z;

	p->v.f[4]   = vel.x;
	p->v.f[5]   = vel.y;
	p->v.f[6]   = vel.z;

	packetQueue(p,23,7*4,c);
}

void msgFxBeamBlaster(uint c, const vec pa, const vec pb, float beamSize, float damageMultiplier){
	packet *p = &packetBuffer;

	p->v.f[0] = pa.x;
	p->v.f[1] = pa.y;
	p->v.f[2] = pa.z;

	p->v.f[3] = pb.x;
	p->v.f[4] = pb.y;
	p->v.f[5] = pb.z;

	p->v.f[6] = beamSize;
	p->v.f[7] = damageMultiplier;

	packetQueueExcept(p,24,8*4,c);
}

void msgItemDropUpdate(uint c, const vec pos, const vec vel, u16 i, u16 len, u16 itemID, u16 amount){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = len;

	p->v.u16[2] = itemID;
	p->v.i16[3] = amount;

	p->v.f[2]   = pos.x;
	p->v.f[3]   = pos.y;
	p->v.f[4]   = pos.z;

	p->v.f[5]   = vel.x;
	p->v.f[6]   = vel.y;
	p->v.f[7]   = vel.z;

	packetQueue(p,25,8*4,c);
}

void msgBeingGotHit(i16 hp, u16 cause, being target, being culprit){
	packet *p = &packetBuffer;

	p->v.i16[0] = hp;
	p->v.u16[1] = cause;
	p->v.u32[1] = target;
	p->v.u32[2] = culprit;

	packetQueue(p,8,3*4,-1);
}

void msgBeingDamage(uint c, i16 hp, u16 cause, being target, being culprit, const vec pos){
	packet *p = &packetBuffer;

	p->v.i16[0] = hp;
	p->v.u16[1] = cause;

	p->v.u32[1] = target;
	p->v.u32[2] = culprit;
	p->v.f[3]   = pos.x;
	p->v.f[4]   = pos.y;
	p->v.f[5]   = pos.z;

	packetQueue(p,26,6*4,c);
}

void msgPlayerSetData(uint c, i16 hp, u16 activeItem, u32 flags){
	packet *p = &packetBuffer;

	p->v.i16[0] = hp;
	p->v.u16[1] = activeItem;
	p->v.u32[2] = flags;

	packetQueue(p,28,3*4,c);
}

void msgPlayerSetInventory(uint c,const item *itm, size_t itemCount){
	packet *p = &packetBuffer;
	for(uint i=0;i<itemCount;i++){
		p->v.u16[(i<<1)  ] = itm[i].ID;
		p->v.i16[(i<<1)+1] = itm[i].amount;
	}
	packetQueue(p,29,itemCount*4,c);
}
