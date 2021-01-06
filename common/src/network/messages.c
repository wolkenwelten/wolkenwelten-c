#include "messages.h"

#include "../common.h"
#include "../game/animal.h"

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

void msgSetTime( int c, u32 time){
	packet *p = &packetBuffer;

	p->v.u32[0] = time;

	packetQueue(p,9,4,c);
}

void msgItemDropNew(uint c, const vec pos, const vec vel, const item *itm){
	packet *p = &packetBuffer;

	p->v.f[0] = pos.x;
	p->v.f[1] = pos.y;
	p->v.f[2] = pos.z;

	p->v.f[3] = vel.x;
	p->v.f[4] = vel.y;
	p->v.f[5] = vel.z;

	p->v.u16[12] = itm->ID;
	p->v.i16[13] = itm->amount;

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

void msgPickupItem(uint c, const item itm){
	packet *p = &packetBuffer;

	p->v.u16[0] = itm.ID;
	p->v.i16[1] = itm.amount;

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

void msgItemDropUpdate(uint c, const vec pos, const vec vel, const item *itm, u16 i, u16 len){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = len;

	p->v.u16[2] = itm->ID;
	p->v.i16[3] = itm->amount;

	p->v.f[2]   = pos.x;
	p->v.f[3]   = pos.y;
	p->v.f[4]   = pos.z;

	p->v.f[5]   = vel.x;
	p->v.f[6]   = vel.y;
	p->v.f[7]   = vel.z;

	packetQueue(p,25,8*4,c);
}

void msgBeingGotHit(i16 hp, u8 cause,float knockbackMult, being target, being culprit){
	packet *p = &packetBuffer;

	p->v.i16[0] = hp;

	p->v.u8[2]  = cause;
	p->v.u8[3]  = knockbackMult * 16.f;

	p->v.u32[1] = target;
	p->v.u32[2] = culprit;

	packetQueue(p,8,3*4,-1);
}

void msgBeingDamage(uint c, i16 hp, u8 cause, float knockbackMult, being target, being culprit, const vec pos){
	packet *p = &packetBuffer;

	p->v.i16[0] = hp;

	p->v.u8[2]  = cause;
	p->v.u8[3]  = knockbackMult * 16.f;

	p->v.u32[1] = target;
	p->v.u32[2] = culprit;

	p->v.f[3]   = pos.x;
	p->v.f[4]   = pos.y;
	p->v.f[5]   = pos.z;

	packetQueue(p,26,6*4,c);
}

void msgPlayerSetData(uint c, i16 hp, u16 activeItem, u32 flags, u16 id){
	packet *p = &packetBuffer;

	p->v.i16[0] = hp;
	p->v.u16[1] = activeItem;
	p->v.u16[2] = id;
	p->v.u32[2] = flags;

	packetQueue(p,28,4*4,c);
}

void msgPlayerSetInventory(uint c,const item *itm, size_t itemCount){
	packet *p = &packetBuffer;
	for(uint i=0;i<itemCount;i++){
		p->v.u16[(i<<1)  ] = itm[i].ID;
		p->v.i16[(i<<1)+1] = itm[i].amount;
	}
	packetQueue(p,29,itemCount*4,c);
}

void msgPingPong(uint c){
	packet *p = &packetBuffer;
	packetQueue(p,33,0,c);
}

void msgAnimalDied(uint c, const animal *a){
	packet *p = &packetBuffer;

	p->v.u8[0]  = a->type;
	p->v.u8[1]  = a->age;
	p->v.u8[2]  = 0;
	p->v.u8[3]  = 0;

	p->v.f[1]   = a->pos.x;
	p->v.f[2]   = a->pos.y;
	p->v.f[3]   = a->pos.z;
	p->v.u32[4] = animalGetBeing(a);

	packetQueue(p,34,5*4,c);
}

void msgPlayerSetEquipment(uint c,const item *itm, size_t itemCount){
	packet *p = &packetBuffer;
	for(uint i=0;i<itemCount;i++){
		p->v.u16[(i<<1)  ] = itm[i].ID;
		p->v.i16[(i<<1)+1] = itm[i].amount;
	}
	packetQueue(p,35,itemCount*4,c);
}

void msgItemDropPickup(uint c, uint i){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;

	packetQueue(p,36,2,c);
}

void msgRopeUpdate(uint c, uint i, rope *r){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = r->flags;
	p->v.u32[1] = r->a;
	p->v.u32[2] = r->b;
	p->v.f  [3] = r->length;

	packetQueue(p,37,4*4,c);
}

void msgFxBeamBlastHit(uint c, const vec pos, u16 size, u16 style){
	packet *p = &packetBuffer;

	p->v.u16[0] = style;
	p->v.u16[1] = size;
	p->v.f  [1] = pos.x;
	p->v.f  [2] = pos.y;
	p->v.f  [3] = pos.z;

	packetQueue(p,39,4*4,c);
}

void msgFireUpdate(uint c, u16 i, u16 count, u16 x, u16 y, u16 z, i16 strength){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = count;

	p->v.u16[2] = x;
	p->v.u16[3] = y;
	p->v.u16[4] = z;
	p->v.i16[5] = strength;

	packetQueue(p,40,6*2,c);
}

void msgWaterUpdate(uint c, u16 i, u16 count, u16 x, u16 y, u16 z, i16 amount){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = count;

	p->v.u16[2] = x;
	p->v.u16[3] = y;
	p->v.u16[4] = z;
	p->v.i16[5] = amount;

	packetQueue(p,41,6*2,c);
}

void msgLispSExpr(uint c, u8 id, const char *str){
	packet *p  = &packetBuffer;
	int len    = strnlen(str,4192);
	p->v.u8[0] = id;
	memcpy(&p->v.u8[1],str,len);
	p->v.u8[len+1] = 0;
	packetQueue(p,42,alignedLen(len+2),c);
}


char *messageNames[256] = {
	"keepalive", // 0
	"playerPos",
	"requestChungus",
	"placeBlock",
	"mineBlock",
	"goodbye", // 5
	"blockMiningUpdate",
	"setChungusLoaded",
	"beingGotHit",
	"setTime",
	"itemDropNew", // 10
	"grenadeNew",
	"beamblast",
	"playerMoveDelta",
	"charaterName",
	"playerPos", // 15
	"chatMsg",
	"dyingMsg",
	"chunkData",
	"setPlayerCount",
	"playerPickupItem", // 20
	"itemDropDel",
	"explode",
	"grenadeUpdate",
	"fxBeamBlaster",
	"msgItemDropUpdate", // 25
	"msgPlayerDamage",
	"--- UNUSED ---",
	"characterSetData",
	"characterSetInventory",
	"animalSync",  // 30
	"dirtyChunk",
	"animalDmg",
	"pingPong",
	"fxAnimalDied",
	"characterSetEquipment", // 35
	"itemDropPickup",
	"ropeUpdate",
	"projectileUpdate",
	"fxProjectileHit",
	"fireRecvUpdate",  // 40
	"waterRecvUpdate",
	"lispRecvSExpr",
	"weatherRecvUpdate",
	"rainRecvUpdate"
};

const char *networkGetMessageName(uint i){
	char *ret = messageNames[i];
	if(i == 0xFF){
		return "LZ4 Message";
	}
	if(ret == NULL){
		ret ="Unknown Message";
	}
	return ret;
}
