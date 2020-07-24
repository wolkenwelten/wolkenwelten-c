#include "messages.h"
#include "packet.h"

#ifndef __MINGW32__
#include <alloca.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint8_t packetBuffer[1024];

void msgRequestPlayerSpawnPos(){
	packet *p = (packet *)packetBuffer;
	packetQueueToServer(p,1,0);
}

void msgRequestChungus(int x, int y, int z){
	packet *p = (packet *)packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	packetQueueToServer(p,2,3*4);
}

void msgPlaceBlock(int x, int y, int z, uint8_t b){
	packet *p = (packet *)packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	p->val.i[3] = b;
	packetQueueToServer(p,3,4*4);
}

void msgMineBlock(int x, int y, int z, uint8_t b){
	packet *p = (packet *)packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	p->val.i[3] = b;
	packetQueue(p,4,4*4,-1);
}

void msgGoodbye(){
	packet *p = (packet *)packetBuffer;
	packetQueueToServer(p,5,0);
}

void msgBlockMiningUpdate(int c, uint16_t x, uint16_t y, uint16_t z, uint16_t damage, int count, int i){
	packet *p = (packet *)packetBuffer;
	p->val.i[0] = (x & 0xFFFF) | ((y      & 0xFFFF)<<16) ;
	p->val.i[1] = (z & 0xFFFF) | ((damage & 0xFFFF)<<16) ;
	p->val.i[2] = count;
	p->val.i[3] = i;
	packetQueue(p,6,4*4,c);
}

void msgSendChungusComplete(int c, int x, int y, int z){
	packet *p = (packet *)packetBuffer;
	p->val.i[0] = x;
	p->val.i[1] = y;
	p->val.i[2] = z;
	packetQueue(p,7,3*4,c);
}

void msgCharacterGotHit(int c,float pwr){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = pwr;
	p->val.i[1] = c;
	packetQueueExcept(p,8,2*4,c);
}

void msgPlayerJoinSendName(const char *name){
	packet *p = (packet *)packetBuffer;
	strncpy((char *)p->val.c,name,28);
	p->val.c[27]=0;
	packetQueueToServer(p,9,28);
}

void msgItemDropNew(float x, float y, float z, float vx, float vy, float vz, int ID, int amount){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = x;
	p->val.f[1] = y;
	p->val.f[2] = z;
	p->val.f[3] = vx;
	p->val.f[4] = vy;
	p->val.f[5] = vz;
	p->val.i[6] = ID;
	p->val.i[7] = amount;
	packetQueueToServer(p,10,8*4);
}

void msgNewGrenade(float x, float y, float z, float yaw, float pitch, float roll, float pwr){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = x;
	p->val.f[1] = y;
	p->val.f[2] = z;
	p->val.f[3] = yaw;
	p->val.f[4] = pitch;
	p->val.f[5] = roll;
	p->val.f[6] = pwr;
	packetQueueToServer(p,11,7*4);
}

void msgBeamBlast(float x, float y, float z, float yaw, float pitch, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = x;
	p->val.f[1] = y;
	p->val.f[2] = z;
	p->val.f[3] = yaw;
	p->val.f[4] = pitch;
	p->val.f[5] = beamSize;
	p->val.f[6] = damageMultiplier;
	p->val.f[7] = recoilMultiplier;
	p->val.i[8] = hitsLeft;
	packetQueueToServer(p,12,9*4);
}

void msgPlayerMove(int c, float dvx, float dvy, float dvz, float dyaw, float dpitch, float droll){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = dvx;
	p->val.f[1] = dvy;
	p->val.f[2] = dvz;
	p->val.f[3] = dyaw;
	p->val.f[4] = dpitch;
	p->val.f[5] = droll;
	p->val.i[6] = c;
	packetQueue(p,13,7*4,c);
}

void msgCharacterHit(int c, float x, float y, float z, float yaw, float pitch, float roll, float pwr){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = x;
	p->val.f[1] = y;
	p->val.f[2] = z;
	p->val.f[3] = yaw;
	p->val.f[4] = pitch;
	p->val.f[5] = roll;
	p->val.f[6] = pwr;
	p->val.i[7] = c;
	packetQueueExcept(p,14,8*4,c);
}

// 15 = parsePlayerPos ???

// 16 = parseChatMsg ???

// 17 = parseDyingMsg ???

// 18 = chunkData ???

void msgSetPlayerCount(int playerLeaving, int playerMax){
	packet *p = (packet *)packetBuffer;
	p->val.u[0] = playerMax;
	p->val.u[1] = playerLeaving;
	packetQueue(p,19,4*4,-1);
}

void msgPickupItem(int c, uint16_t ID, uint16_t amount){
	packet *p = (packet *)packetBuffer;
	p->val.s[0] = ID;
	p->val.s[1] = amount;
	packetQueue(p,20,2*2,c);
}

void msgItemDropUpdatePlayer(int c, float x, float y, float z, float aniStep, int ID, int amount, int count, int i){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = x;
	p->val.f[1] = y;
	p->val.f[2] = z;
	p->val.f[3] = aniStep;
	p->val.i[4] = ID;
	p->val.i[5] = amount;
	p->val.i[6] = count;
	p->val.i[7] = i;
	packetQueue(p,21,4*8,c);
}

void msgGrenadeExplode(float x, float y, float z,float pwr, int style){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = x;
	p->val.f[1] = y;
	p->val.f[2] = z;
	p->val.f[3] = pwr;
	p->val.i[4] = style;
	packetQueue(p,22,5*4,-1);
}

void msgGrenadeUpdate(int c, float x, float y, float z, float vx, float vy, float vz, int count, int i){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = x;
	p->val.f[1] = y;
	p->val.f[2] = z;
	p->val.f[3] = vx;
	p->val.f[4] = vy;
	p->val.f[5] = vz;
	p->val.i[6] = count;
	p->val.i[7] = i;
	packetQueue(p,23,8*4,c);
}

void msgFxBeamBlaster(int c, float x1, float y1, float z1, float x2, float y2, float z2, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft){
	packet *p = (packet *)packetBuffer;
	p->val.f[0] = x1;
	p->val.f[1] = y1;
	p->val.f[2] = z1;
	p->val.f[3] = x2;
	p->val.f[4] = y2;
	p->val.f[5] = z2;
	p->val.f[6] = beamSize;
	p->val.f[7] = damageMultiplier;
	p->val.f[8] = recoilMultiplier;
	p->val.i[9] = hitsLeft;

	p->val.i[10] = c;
	packetQueueExcept(p,24,8*4,c);
	p->val.i[10] = 65535;
	packetQueue(p,24,11*4,c);
}
