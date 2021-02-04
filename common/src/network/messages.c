/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "messages.h"

#include "../common.h"
#include "../game/animal.h"

#include <string.h>

packet packetBuffer;

void msgNOP(uint len){
	packet *p = &packetBuffer;
	memset(p->v.u8,0,len);
	packetQueueToServer(p,msgtKeepalive,len);
}

void msgRequestPlayerSpawnPos(){
	packet *p = &packetBuffer;
	packetQueueToServer(p,msgtRequestSpawnPos,0);
}

void msgPlayerSetPos(uint c, const vec pos, const vec rot){
	packet *p = &packetBuffer;

	p->v.f[0] = pos.x;
	p->v.f[1] = pos.y;
	p->v.f[2] = pos.z;

	p->v.f[3] = rot.yaw;
	p->v.f[4] = rot.pitch;
	p->v.f[5] = rot.roll;

	packetQueue(p,msgtPlayerPos,6*4,c);
}

void msgRequestChungus(u8 x, u8 y, u8 z){
	packet *p = &packetBuffer;

	p->v.u8[0] = x;
	p->v.u8[1] = y;
	p->v.u8[2] = z;
	p->v.u8[3] = 0;

	packetQueueToServer(p,msgtRequestChungus,4);
}

void msgUnsubChungus(u8 x, u8 y, u8 z){
	packet *p = &packetBuffer;

	p->v.u8[0] = x;
	p->v.u8[1] = y;
	p->v.u8[2] = z;
	p->v.u8[3] = 0;

	packetQueueToServer(p,msgtChungusUnsub,4);
}

void msgDirtyChunk(u16 x, u16 y, u16 z){
	packet *p = &packetBuffer;
	p->v.u16[0] = x;
	p->v.u16[1] = y;
	p->v.u16[2] = z;
	p->v.u16[3] = 0;
	packetQueueToServer(p,msgtDirtyChunk,2*4);
}

void msgPlaceBlock(u16 x, u16 y, u16 z, u8 b){
	packet *p = &packetBuffer;
	p->v.u16[0] = x;
	p->v.u16[1] = y;
	p->v.u16[2] = z;
	p->v.u16[3] = b;
	packetQueueToServer(p,msgtPlaceBlock,2*4);
}

void msgMineBlock(u16 x, u16 y, u16 z, u8 b, u8 cause){
	packet *p = &packetBuffer;
	p->v.u16[0] = x;
	p->v.u16[1] = y;
	p->v.u16[2] = z;
	p->v.u8[6]  = b;
	p->v.u8[7]  = cause;
	packetQueue(p,msgtMineBlock,2*4,-1);
}

void msgGoodbye(){
	packet *p = &packetBuffer;
	packetQueueToServer(p,msgtGoodbye,0);
}

void msgBlockMiningUpdate(uint c, u16 x, u16 y, u16 z, i16 damage, u16 count, u16 i){
	packet *p   = &packetBuffer;

	p->v.u16[0] = x;
	p->v.u16[1] = y;
	p->v.u16[2] = z;
	p->v.i16[3] = damage;
	p->v.u16[4] = i;
	p->v.u16[5] = count;

	packetQueue(p,msgtBlockMiningUpdate,6*2,c);
}

void msgSendChungusComplete(uint c, u8 x, u8 y, u8 z){
	packet *p = &packetBuffer;

	p->v.u8[0] = x;
	p->v.u8[1] = y;
	p->v.u8[2] = z;
	p->v.u8[3] = 0;

	packetQueue(p,msgtSetChungusLoaded,4,c);
}

void msgSetTime( int c, u32 time){
	packet *p = &packetBuffer;

	p->v.u32[0] = time;

	packetQueue(p,msgtSetTime,4,c);
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

	packetQueue(p,msgtItemDropNew,8*4,c);
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

	packetQueueToServer(p,msgtGrenadeNew,9*4);
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

	packetQueueToServer(p,msgtBeamblast,9*4);
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

	packetQueue(p,msgtPlayerMoveDelta,7*4,c);
}

void msgPlayerName(uint c, u16 i, const char *name){
	packet *p = &packetBuffer;
	p->v.u16[0] = i;
	strncpy((char *)&p->v.u8[2],name,31);
	p->v.u8[33] = 0;
	packetQueue(p,msgtCharacterName,36,c);
}

void msgSetPlayerCount(u16 playerLeaving, u16 playerMax){
	packet *p = &packetBuffer;

	p->v.u16[0] = playerMax;
	p->v.u16[1] = playerLeaving;

	packetQueue(p,msgtSetPlayerCount,2*2,-1);
}

void msgPickupItem(uint c, const item itm){
	packet *p = &packetBuffer;

	p->v.u16[0] = itm.ID;
	p->v.i16[1] = itm.amount;

	packetQueue(p,msgtPlayerPickupItem,2*2,c);
}

void msgGrenadeExplode(const vec pos,float pwr, u16 style){
	packet *p = &packetBuffer;

	p->v.f[0] = pos.x;
	p->v.f[1] = pos.y;
	p->v.f[2] = pos.z;

	p->v.u16[6] = (u16)(pwr*256.f);
	p->v.u16[7] = style;

	packetQueue(p,msgtExplode,4*4,-1);
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

	packetQueue(p,msgtGrenadeUpdate,7*4,c);
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

	packetQueueExcept(p,msgtFxBeamBlaster,8*4,c);
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

	packetQueue(p,msgtItemDropUpdate,8*4,c);
}

void msgBeingGotHit(i16 hp, u8 cause,float knockbackMult, being target, being culprit){
	packet *p = &packetBuffer;

	p->v.i16[0] = hp;

	p->v.u8[2]  = cause;
	p->v.u8[3]  = knockbackMult * 16.f;

	p->v.u32[1] = target;
	p->v.u32[2] = culprit;

	packetQueue(p,msgtBeingGotHit,3*4,-1);
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

	packetQueue(p,msgtBeingDamage,6*4,c);
}

void msgPlayerSetData(uint c, i16 hp, u16 activeItem, u32 flags, u16 id){
	packet *p = &packetBuffer;

	p->v.i16[0] = hp;
	p->v.u16[1] = activeItem;
	p->v.u16[2] = id;
	p->v.u32[2] = flags;

	packetQueue(p,msgtCharacterSetData,4*4,c);
}

void msgPlayerSetInventory(uint c,const item *itm, size_t itemCount){
	packet *p = &packetBuffer;
	for(uint i=0;i<itemCount;i++){
		p->v.u16[(i<<1)  ] = itm[i].ID;
		p->v.i16[(i<<1)+1] = itm[i].amount;
	}
	packetQueue(p,msgtCharacterSetInventory,itemCount*4,c);
}

void msgPingPong(uint c){
	packet *p = &packetBuffer;
	packetQueue(p,msgtPingPong,0,c);
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

	packetQueue(p,msgtFxAnimalDied,5*4,c);
}

void msgPlayerSetEquipment(uint c,const item *itm, size_t itemCount){
	packet *p = &packetBuffer;
	for(uint i=0;i<itemCount;i++){
		p->v.u16[(i<<1)  ] = itm[i].ID;
		p->v.i16[(i<<1)+1] = itm[i].amount;
	}
	packetQueue(p,msgtCharacterSetEquipment,itemCount*4,c);
}

void msgItemDropPickup(uint c, uint i){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;

	packetQueue(p,msgtItemDropPickup,2,c);
}

void msgRopeUpdate(uint c, uint i, rope *r){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = r->flags;
	p->v.u32[1] = r->a;
	p->v.u32[2] = r->b;
	p->v.f  [3] = r->length;

	packetQueue(p,msgtRopeUpdate,4*4,c);
}

void msgFxBeamBlastHit(uint c, const vec pos, u16 size, u16 style){
	packet *p = &packetBuffer;

	p->v.u16[0] = style;
	p->v.u16[1] = size;
	p->v.f  [1] = pos.x;
	p->v.f  [2] = pos.y;
	p->v.f  [3] = pos.z;

	packetQueue(p,msgtFxProjectileHit,4*4,c);
}

void msgFireUpdate(uint c, u16 i, u16 count, u16 x, u16 y, u16 z, i16 strength){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = count;

	p->v.u16[2] = x;
	p->v.u16[3] = y;
	p->v.u16[4] = z;
	p->v.i16[5] = strength;

	packetQueue(p,msgtFireRecvUpdate,6*2,c);
}

void msgLispSExpr(uint c, u8 id, const char *str){
	packet *p  = &packetBuffer;
	int len    = strnlen(str,4192);
	p->v.u8[0] = id;
	memcpy(&p->v.u8[1],str,len);
	p->v.u8[len+1] = 0;
	packetQueue(p,msgtLispRecvSExpr,alignedLen(len+2),c);
}


const char *networkGetMessageName(uint i){
	switch((messageType)i){
	case msgtKeepalive:
		return "keepalive";
	case msgtPlayerPos:
		return "playerPos";
	case msgtRequestChungus:
		return "requestChungus";
	case msgtPlaceBlock:
		return "placeBlock";
	case msgtMineBlock:
		return "mineBlock";
	case msgtGoodbye:
		return "goodbye";
	case msgtBlockMiningUpdate:
		return "blockMiningUpdate";
	case msgtSetChungusLoaded:
		return "setChungusLoaded";
	case msgtBeingGotHit:
		return "beingGotHit";
	case msgtSetTime:
		return "setTime";
	case msgtItemDropNew:
		return "itemDropNew";
	case msgtGrenadeNew:
		return "grenadeNew";
	case msgtBeamblast:
		return "beamblast";
	case msgtPlayerMoveDelta:
		return "playerMoveDelta";
	case msgtCharacterUpdate:
		return "charaterUpdate";
	case msgtCharacterName:
		return "charaterName";
	case msgtChatMsg:
		return "chatMsg";
	case msgtDyingMsg:
		return "dyingMsg";
	case msgtChunkData:
		return "chunkData";
	case msgtSetPlayerCount:
		return "setPlayerCount";
	case msgtPlayerPickupItem:
		return "playerPickupItem";
	case msgtItemDropDel:
		return "itemDropDel";
	case msgtExplode:
		return "explode";
	case msgtGrenadeUpdate:
		return "grenadeUpdate";
	case msgtFxBeamBlaster:
		return "fxBeamBlaster";
	case msgtItemDropUpdate:
		return "msgItemDropUpdate";
	case msgtBeingDamage:
		return "msgBeingDamage";
	case msgtChungusUnsub:
		return "chungusUnsubPlayer";
	case msgtCharacterSetData:
		return "characterSetData";
	case msgtCharacterSetInventory:
		return "characterSetInventory";
	case msgtAnimalSync:
		return "animalSync";
	case msgtDirtyChunk:
		return "dirtyChunk";
	case msgtAnimalDmg:
		return "animalDmg";
	case msgtPingPong:
		return "pingPong";
	case msgtFxAnimalDied:
		return "fxAnimalDied";
	case msgtCharacterSetEquipment:
		return "characterSetEquipment";
	case msgtItemDropPickup:
		return "itemDropPickup";
	case msgtRopeUpdate:
		return "ropeUpdate";
	case msgtProjectileUpdate:
		return "projectileUpdate";
	case msgtFxProjectileHit:
		return "fxProjectileHit";
	case msgtFireRecvUpdate:
		return "fireRecvUpdate";
	case msgtLispRecvSExpr:
		return "lispRecvSExpr";
	case msgtWeatherRecvUpdate:
		return "weatherRecvUpdate";
	case msgtRainRecvUpdate:
		return "rainRecvUpdate";
	case msgtThrowableRecvUpdates:
		return "throwableRecvUpdate";
	case msgtRequestSpawnPos:
		return "RequestSpawnPos";
	case msgtLZ4:
		return "LZ4 Message";
	}
}
