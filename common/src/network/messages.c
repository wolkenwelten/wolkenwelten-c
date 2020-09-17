#include "messages.h"
#include "packet.h"
#include "../common.h"

#include <string.h>
#include <time.h>

packet packetBuffer;

void msgRequestPlayerSpawnPos(){
	packet *p = &packetBuffer;
	packetQueueToServer(p,1,0);
}

void msgPlayerSetPos(int c, const vec pos, const vec rot){
	packet *p = &packetBuffer;
	p->val.f[0] = pos.x;
	p->val.f[1] = pos.y;
	p->val.f[2] = pos.z;
	p->val.f[3] = rot.yaw;
	p->val.f[4] = rot.pitch;
	p->val.f[5] = rot.roll;
	packetQueue(p,1,6*4,c);
}

void msgRequestChungus(int x, int y, int z){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	packetQueueToServer(p,2,3*4);
}

void msgPlaceBlock(int x, int y, int z, uint8_t b){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	p->val.i[3] = b;
	packetQueueToServer(p,3,4*4);
}

void msgMineBlock(int x, int y, int z, uint8_t b){
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

void msgBlockMiningUpdate(int c, uint16_t x, uint16_t y, uint16_t z, uint16_t damage, int count, int i){
	packet *p = &packetBuffer;
	p->val.i[0] = (x & 0xFFFF) | ((y      & 0xFFFF)<<16) ;
	p->val.i[1] = (z & 0xFFFF) | ((damage & 0xFFFF)<<16) ;
	p->val.i[2] = count;
	p->val.i[3] = i;
	packetQueue(p,6,4*4,c);
}

void msgSendChungusComplete(int c, int x, int y, int z){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	packetQueue(p,7,3*4,c);
}

void msgCharacterGotHit(int c,int pwr){
	packet *p = &packetBuffer;
	p->val.i[0] = pwr;
	p->val.i[1] = c;
	packetQueueExcept(p,8,2*4,c);
}

void msgItemDropNew(int c, const vec pos, const vec vel, const item *itm){
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

void msgPlayerMove(int c, const vec dpos, const vec drot){
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

void msgCharacterHit(int c, const vec pos, const vec rot, int pwr){
	packet *p = &packetBuffer;
	p->val.f[0] = pos.x;
	p->val.f[1] = pos.y;
	p->val.f[2] = pos.z;
	p->val.f[3] = rot.yaw;
	p->val.f[4] = rot.pitch;
	p->val.f[5] = rot.roll;
	p->val.i[6] = pwr;
	p->val.i[7] = c;
	packetQueueExcept(p,14,8*4,c);
}

// 15 = parsePlayerPos ???

// 16 = parseChatMsg ???

// 17 = parseDyingMsg ???

// 18 = chunkData ???

void msgSetPlayerCount(int playerLeaving, int playerMax){
	packet *p = &packetBuffer;
	p->val.u[0] = playerMax;
	p->val.u[1] = playerLeaving;
	packetQueue(p,19,2*4,-1);
}

void msgPickupItem(int c, uint16_t ID, uint16_t amount){
	packet *p = &packetBuffer;
	p->val.s[0] = ID;
	p->val.s[1] = amount;
	packetQueue(p,20,2*2,c);
}

void msgGrenadeExplode(const vec pos,,float pwr, int style){
	packet *p = &packetBuffer;
	p->val.f[0] = pos.x;
	p->val.f[1] = pos.y;
	p->val.f[2] = pos.z;
	p->val.f[3] = pwr;
	p->val.i[4] = style;
	packetQueue(p,22,5*4,-1);
}

void msgGrenadeUpdate(int c, const vec pos, const vec vel, int count, int i){
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

void msgFxBeamBlaster(int c, const vec pa, const vec pb, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft){
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
	p->val.i[9] = hitsLeft;

	p->val.i[10] = c;
	packetQueueExcept(p,24,11*4,c);
	p->val.i[10] = 65535;
	packetQueue(p,24,11*4,c);
}

void msgItemDropUpdate(int c, const vec pos, const vec vel, uint16_t i, uint16_t len, const item *itm){
	packet *p = &packetBuffer;

	p->val.s[0] = i;
	p->val.s[1] = len;

	p->val.s[2] = itm->ID;
	p->val.s[3] = itm->amount;

	p->val.f[2] = pos.x;
	p->val.f[3] = pos.y;
	p->val.f[4] = pos.z;
	p->val.f[5] = vel.x;
	p->val.f[6] = vel.y;
	p->val.f[7] = vel.z;

	packetQueue(p,25,8*4,c);
}

void msgPlayerDamage(int c, int hp){
	packet *p = &packetBuffer;
	p->val.i[0]=hp;
	packetQueue(p,26,4,c);
}

void msgUnsubChungus(int x, int y, int z){
	packet *p = &packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	packetQueueToServer(p,27,3*4);
}

void msgPlayerSetData(int c, int hp, int activeItem, uint32_t flags){
	packet *p = &packetBuffer;
	p->val.i[0] = hp;
	p->val.i[1] = activeItem;
	p->val.u[2] = flags;
	packetQueue(p,28,3*4,c);
}

void msgPlayerSetInventory(int c,const item *itm, size_t itemCount){
	packet *p = &packetBuffer;
	for(unsigned int i=0;i<itemCount;i++){
		p->val.s[(i<<1)  ] = itm[i].ID;
		p->val.s[(i<<1)+1] = itm[i].amount;
	}
	packetQueue(p,29,itemCount*4,c);
}
