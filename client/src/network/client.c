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

#include "client.h"
#include "../main.h"
#include "../gfx/effects.h"
#include "../gui/menu.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/fire.h"
#include "../game/grenade.h"
#include "../game/itemDrop.h"
#include "../game/rain.h"
#include "../game/rope.h"
#include "../game/projectile.h"
#include "../game/throwable.h"
#include "../game/time.h"
#include "../game/weather.h"
#include "../misc/lisp.h"
#include "../misc/options.h"
#include "../sdl/sdl.h"
#include "../network/chat.h"
#include "../../../common/src/misc/lz4.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"
#include "../voxel/bigchungus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

uint connectionState = 0;
uint lastPing        = 0;
uint lastLatency     = 0;
uint recvBufLen      = 0;
u8 recvBuf[1<<22];

uint sendBufSent = 0;
uint sendBufLen  = 0;
u8 sendBuf[1<<16];

size_t sentBytesCurrentSession             = 0;
size_t recvBytesCurrentSession             = 0;
size_t recvUncompressedBytesCurrentSession = 0;

int serverPort        = 6309;
pid_t singlePlayerPID = 0;
int connectionTries   = 0;

#ifdef __EMSCRIPTEN__
#include "client_wasm.h"
#elif defined __MINGW32__
#include "client_win.h"
#else
#include "client_bsd.h"
#endif

void msgSendPlayerPos(){
	static int inventoryCountDown=0;
	int pLen = 15*4;
	if(player == NULL){return;}
	if(--inventoryCountDown <= 0){
		msgPlayerSetInventory(-1,player->inventory,40);
		msgPlayerSetEquipment(-1,player->equipment, 3);
		inventoryCountDown = 60;
	}
	packet *p = &packetBuffer;

	p->v.f[ 0] = player->pos.x;
	p->v.f[ 1] = player->pos.y;
	p->v.f[ 2] = player->pos.z;

	p->v.f[ 3] = player->rot.yaw;
	p->v.f[ 4] = player->rot.pitch;
	p->v.f[ 5] = player->yoff;

	p->v.f[ 6] = player->vel.x;
	p->v.f[ 7] = player->vel.y;
	p->v.f[ 8] = player->vel.z;

	p->v.u32[ 9] = player->flags;

	p->v.u16[20] = player->blockMiningX;
	p->v.u16[21] = player->blockMiningY;
	p->v.u16[22] = player->blockMiningZ;
	p->v.u16[23] = player->hp;

	p->v.u16[24] = player->activeItem;
	p->v.u16[25] = player->animationIndex;
	p->v.u16[26] = player->animationTicksMax;
	p->v.u16[27] = player->animationTicksLeft;

	if(player->hook != NULL){
		p->v.f[15] = player->hook->ent->pos.x;
		p->v.f[16] = player->hook->ent->pos.y;
		p->v.f[17] = player->hook->ent->pos.z;
		pLen = 18*4;
	}
	packetQueueToServer(p,msgtCharacterUpdate,pLen);
}

void decompressPacket(const packet *p){
	static u8 buf[1<<22];
	u8 *t;
	int len = LZ4_decompress_safe((const char *)&p->v.u8, (char *)buf, packetLen(p), sizeof(buf));
	if(len <= 0){
		fprintf(stderr,"Decompression return %i\n",len);
		exit(1);
	}
	for(t=buf;(t-buf)<len;t+=alignedLen(4+packetLen((packet *)t))){
		clientParsePacket((packet *)t);
	}
}

void dispatchBeingGotHit(const packet *p){
	const being target  = p->v.u32[1];

	switch(beingType(target)){
	default:
		fprintf(stderr,"dispatchBeingGotHit: Unknown being %x",target);
		break;
	case BEING_CHARACTER:
		characterGotHitPacket(p);
		break;
	case BEING_ANIMAL:
		animalGotHitPacket(p);
		break;
	}
}

static void handlePingPong(){
	uint curPing = getTicks();
	lastLatency = ((curPing - lastPing) + lastLatency)/2;
	lastPing = curPing;
}

void clientParsePacket(const packet *p){
	const int pLen  = packetLen(p);
	const int pType = packetType(p);
	if(packetFalseChecksum(p)){
		fprintf(stderr,"[CLI] CHECKSUM WRONG!!! T:%i L:%i\n",pType,pLen);
		return;
	}
	if(pType != 0xFF){
		recvUncompressedBytesCurrentSession += pLen+4;
	}
	nprofAddPacket(pType,pLen);

	switch((messageType)pType){
	case msgtKeepalive:
		break;
	case msgtPlayerPos:
		characterSetPos(player,vecNewP(&p->v.f[0]));
		characterSetRot(player,vecNewP(&p->v.f[3]));
		characterSetVelocity(player,vecZero());
		characterFreeHook(player);
		player->flags &= ~CHAR_SPAWNING;
		break;
	case msgtMineBlock:
		fxBlockBreak(vecNew(p->v.u16[0],p->v.u16[1],p->v.u16[2]),p->v.u8[6],p->v.u8[7]);
		break;
	case msgtBlockMiningUpdate:
		blockMiningUpdateFromServer(p);
		break;
	case msgtSetChungusLoaded:
		worldSetChungusLoaded(p->v.u8[0],p->v.u8[1],p->v.u8[2]);
		break;
	case msgtBeingGotHit:
		dispatchBeingGotHit(p);
		break;
	case msgtSetTime:
		gtimeSetTime(p->v.u32[0]);
		break;
	case msgtPlayerMoveDelta:
		characterMoveDelta(player,p);
		break;
	case msgtCharacterName:
		characterSetName(p);
		break;
	case msgtCharacterUpdate:
		characterUpdatePacket(p);
		break;
	case msgtChatMsg:
		chatParsePacket(p);
		break;
	case msgtChunkData:
		chunkRecvUpdate(p);
		break;
	case msgtSetPlayerCount:
		characterRemovePlayer(p->v.u16[1],p->v.u16[0]);
		break;
	case msgtPlayerPickupItem:
		characterPickupPacket(player,p);
		break;
	case msgtExplode:
		explode(vecNewP(&p->v.f[0]),((float)p->v.u16[6])/256.f,p->v.u16[7]);
		break;
	case msgtGrenadeUpdate:
		grenadeUpdateFromServer(p);
		break;
	case msgtFxBeamBlaster:
		fxBeamBlaster(vecNewP(&p->v.f[0]),vecNewP(&p->v.f[3]),p->v.f[6],p->v.f[7]);
		break;
	case msgtItemDropUpdate:
		itemDropUpdateFromServer(p);
		break;
	case msgtBeingDamage:
		characterDamagePacket(player,p);
		break;
	case msgtCharacterSetData:
		characterSetData(player,p);
		break;
	case msgtCharacterSetInventory:
		characterSetInventoryP(player,p);
		break;
	case msgtAnimalSync:
		animalSyncFromServer(p);
		break;
	case msgtPingPong:
		handlePingPong();
		msgPingPong(-1);
		break;
	case msgtFxAnimalDied:
		fxAnimalDiedPacket(p);
		break;
	case msgtCharacterSetEquipment:
		characterSetEquipmentP(player,p);
		break;
	case msgtRopeUpdate:
		ropeUpdateP(p);
		break;
	case msgtProjectileUpdate:
		projectileRecvUpdate(-1,p);
		break;
	case msgtFxProjectileHit:
		fxProjectileHit(p);
		break;
	case msgtFireRecvUpdate:
		fireRecvUpdate(-1,p);
		break;
	case msgtLispRecvSExpr:
		lispRecvSExpr(p);
		break;
	case msgtWeatherRecvUpdate:
		weatherRecvUpdate(p);
		break;
	case msgtRainRecvUpdate:
		rainRecvUpdate(p);
		break;
	case msgtThrowableRecvUpdates:
		throwableRecvUpdate(p);
		break;

	case msgtLZ4:
		decompressPacket(p);
		break;

	case msgtRequestChungus:
	case msgtPlaceBlock:
	case msgtGoodbye:
	case msgtDyingMsg:
	case msgtRequestSpawnPos:
	case msgtChungusUnsub:
	case msgtItemDropNew:
	case msgtItemDropDel:
	case msgtItemDropPickup:
	case msgtGrenadeNew:
	case msgtBeamblast:
	case msgtDirtyChunk:
	case msgtAnimalDmg:
		fprintf(stderr,"%s[%u] received from server, which should never happen\n",networkGetMessageName(pType),pType);
		break;
	}
}

void clientParse(){
	uint off=0;
	if(recvBufLen == 0){return;}

	for(int max=4096;max > 0;--max){
		if(off >= recvBufLen){break;}
		int pLen = packetLen((packet *)(recvBuf+off));
		if((off+alignedLen(pLen+4)) > recvBufLen){
			break;
		}
		clientParsePacket((packet *)(recvBuf+off));
		off += alignedLen(pLen+4);
	}
	if(off < recvBufLen){
		memmove(recvBuf,&recvBuf[off],recvBufLen-off);
	}
	recvBufLen -= off;
}

void clientGreetServer(){
	char introStr[34];
	connectionState = 1;
	#ifdef __EMSCRIPTEN__
		if(singlePlayerPID != 0){queueToServer("NATIVE\r\n\r\n",10);}
	#else
		queueToServer("NATIVE\r\n\r\n",10);
	#endif
	uint len = snprintf(introStr,sizeof(introStr),"%.32s\n",playerName);
	queueToServer(introStr,len);
	clientWrite();
	lastPing = getTicks();
}

void clientTranceive(){
	clientRead();
	clientParse();
	ropeSyncAll();
	clientWrite();
}

void clientGoodbye(){
	if(serverSocket <= 0){return;}
	printf("[CLI] Goodbye \n");
	msgSendPlayerPos();
	msgPlayerSetInventory(-1,player->inventory,40);
	msgPlayerSetEquipment(-1,player->equipment, 3);
	msgGoodbye();
	clientWrite();
}

void queueToServer(const void *data, uint len){
	if((sendBufLen + len) > sizeof(sendBuf)){
		clientWrite();
	}
	memcpy(sendBuf + sendBufLen, data, len);
	sendBufLen += len;
}

void clientFree(){
	clientFreeSpecific();
	singlePlayerPID = 0;
	recvBufLen      = 0;
	sendBufLen      = 0;
	sendBufSent     = 0;
	worldFree();
	characterInit(player);
	chatEmpty();
	connectionState = 0;
}
