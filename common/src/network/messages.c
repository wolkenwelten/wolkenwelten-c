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

#include <stdio.h>
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

void msgPlayerSetPos(int c, const vec pos, const vec rot, const vec vel){
	packet *p = &packetBuffer;

	p->v.f[0] = pos.x;
	p->v.f[1] = pos.y;
	p->v.f[2] = pos.z;

	p->v.f[3] = rot.yaw;
	p->v.f[4] = rot.pitch;
	p->v.f[5] = rot.roll;

	p->v.f[6] = vel.x;
	p->v.f[7] = vel.y;
	p->v.f[8] = vel.z;

	packetQueue(p,msgtPlayerPos,9*4,c);
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

void msgBlockMiningUpdate(int c, u16 x, u16 y, u16 z, i16 damage, u16 count, u16 i){
	packet *p   = &packetBuffer;

	p->v.u16[0] = x;
	p->v.u16[1] = y;
	p->v.u16[2] = z;
	p->v.i16[3] = damage;
	p->v.u16[4] = i;
	p->v.u16[5] = count;

	packetQueue(p,msgtBlockMiningUpdate,6*2,c);
}

void msgSendChungusComplete(int c, u8 x, u8 y, u8 z){
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

void msgItemDropNew(int c, const vec pos, const vec vel, const item *itm){
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

void msgBeingMove(being b, vec dpos, vec dvel){
	packet *p = &packetBuffer;

	p->v.u32[0] = b;

	p->v.f[1] = dpos.x;
	p->v.f[2] = dpos.y;
	p->v.f[3] = dpos.z;

	p->v.f[4] = dvel.x;
	p->v.f[5] = dvel.y;
	p->v.f[6] = dvel.z;


	packetQueue(p,msgtBeingMove,7*4,-1);
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

void msgPlayerMove(int c, const vec dpos, const vec drot){
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

void msgPlayerName(int c, u16 i, const char *name){
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

void msgPickupItem(int c, const item itm){
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

void msgGrenadeUpdate(int c, const grenade *g, u16 i, u16 count){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = count;

	if(g != NULL){
		p->v.f[1]   = g->ent->pos.x;
		p->v.f[2]   = g->ent->pos.y;
		p->v.f[3]   = g->ent->pos.z;

		p->v.f[4]   = g->ent->vel.x;
		p->v.f[5]   = g->ent->vel.y;
		p->v.f[6]   = g->ent->vel.z;

		p->v.f[7]   = g->pwr;
		p->v.i32[8] = g->cluster;
		p->v.f[9]   = g->clusterPwr;
	}else{
		if(i || count){
			fprintf(stderr,"Trying to sync a NULL grenade!\n");
		}
	}

	packetQueue(p,msgtGrenadeUpdate,10*4,c);
}

void msgFxBeamBlaster(int c, const vec pa, const vec pb, float beamSize, float damageMultiplier){
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

void msgItemDropUpdate(int c, const vec pos, const vec vel, const item *itm, u16 i, u16 len, i16 IDPlayer){
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

	p->v.i16[16] = IDPlayer;
	p->v.i16[17] = 0;

	packetQueue(p,msgtItemDropUpdate,9*4,c);
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

void msgBeingDamage(int c, i16 hp, u8 cause, float knockbackMult, being target, being culprit, const vec pos){
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

void msgPlayerSetData(int c, i16 hp, u16 activeItem, u32 flags, u16 id){
	packet *p = &packetBuffer;

	p->v.i16[0] = hp;
	p->v.u16[1] = activeItem;
	p->v.u16[2] = id;
	p->v.u32[2] = flags;

	packetQueue(p,msgtCharacterSetData,4*4,c);
}

void msgPlayerSetInventory(int c,const item *itm, size_t itemCount){
	packet *p = &packetBuffer;
	for(uint i=0;i<itemCount;i++){
		p->v.u16[(i<<1)  ] = itm[i].ID;
		p->v.i16[(i<<1)+1] = itm[i].amount;
	}
	packetQueue(p,msgtCharacterSetInventory,itemCount*4,c);
}

void msgPingPong(int c){
	packet *p = &packetBuffer;
	packetQueue(p,msgtPingPong,0,c);
}

void msgAnimalDied(int c, const animal *a){
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

void msgPlayerSetEquipment(int c,const item *itm, size_t itemCount){
	packet *p = &packetBuffer;
	for(uint i=0;i<itemCount;i++){
		p->v.u16[(i<<1)  ] = itm[i].ID;
		p->v.i16[(i<<1)+1] = itm[i].amount;
	}
	packetQueue(p,msgtCharacterSetEquipment,itemCount*4,c);
}

void msgItemDropPickup(int c, uint i){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;

	packetQueue(p,msgtItemDropPickup,2,c);
}

void msgItemDropBounce(int c, uint i){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;

	packetQueue(p,msgtItemDropBounce,2,c);
}

void msgRopeUpdate(int c, uint i, rope *r){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = r->flags;
	p->v.u32[1] = r->a;
	p->v.u32[2] = r->b;
	p->v.f  [3] = r->length;

	packetQueue(p,msgtRopeUpdate,4*4,c);
}

void msgFxBeamBlastHit(int c, const vec pos, u16 size, u16 style){
	packet *p = &packetBuffer;

	p->v.u16[0] = style;
	p->v.u16[1] = size;
	p->v.f  [1] = pos.x;
	p->v.f  [2] = pos.y;
	p->v.f  [3] = pos.z;

	packetQueue(p,msgtFxProjectileHit,4*4,c);
}

void msgFireUpdate(int c, u16 i, u16 count, u16 x, u16 y, u16 z, i16 strength){
	packet *p = &packetBuffer;

	p->v.u16[0] = i;
	p->v.u16[1] = count;

	p->v.u16[2] = x;
	p->v.u16[3] = y;
	p->v.u16[4] = z;
	p->v.i16[5] = strength;

	packetQueue(p,msgtFireRecvUpdate,6*2,c);
}

void msgLispSExpr(int c, const char *str){
	packet *p  = &packetBuffer;
	int len    = strnlen(str,4192);
	memcpy(p->v.u8,str,len);
	p->v.u8[len] = 0;
	packetQueue(p,msgtLispRecvSExpr,alignedLen(len+1),c);
}

void msgGoodbye(int c){
	packet *p = &packetBuffer;
	packetQueue(p,msgtGoodbye,0,c);
}

void msgLightningStrike(int c, u16 lx, u16 ly, u16 lz, u16 tx, u16 ty, u16 tz, u16 seed){
	packet *p = &packetBuffer;

	p->v.u16[0] = lx;
	p->v.u16[1] = ly;
	p->v.u16[2] = lz;

	p->v.u16[3] = tx;
	p->v.u16[4] = ty;
	p->v.u16[5] = tz;

	p->v.u16[6] = seed;

	packetQueue(p,msgtLightningStrike,alignedLen(14),c);
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
	case msgtBeingMove:
		return "beingMove";
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
	case msgtChunkData:
		return "chunkData";
	case msgtSetPlayerCount:
		return "setPlayerCount";
	case msgtPlayerPickupItem:
		return "playerPickupItem";
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
	case msgtPingPong:
		return "pingPong";
	case msgtFxAnimalDied:
		return "fxAnimalDied";
	case msgtCharacterSetEquipment:
		return "characterSetEquipment";
	case msgtItemDropPickup:
		return "itemDropPickup";
	case msgtItemDropBounce:
		return "itemDropBounce";
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
	case msgtLightningStrike:
		return "lightningStrike";
	case msgtWeatherRecvUpdate:
		return "weatherRecvUpdate";
	case msgtRainRecvUpdate:
		return "rainRecvUpdate";
	case msgtSnowRecvUpdate:
		return "snowRecvUpdate";
	case msgtThrowableRecvUpdates:
		return "throwableRecvUpdate";
	case msgtRequestSpawnPos:
		return "RequestSpawnPos";
	case msgtLZ4:
		return "LZ4 Message";
	}
	return "Unknown";
}
