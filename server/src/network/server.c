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

#include "server.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/beamblast.h"
#include "../game/being.h"
#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/grenade.h"
#include "../game/projectile.h"
#include "../game/throwable.h"
#include "../game/rope.h"
#include "../game/weather.h"
#include "../misc/lisp.h"
#include "../misc/options.h"
#include "../network/server_ws.h"
#include "../persistence/character.h"
#include "../persistence/savegame.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/projectile.h"
#include "../../../common/src/game/time.h"
#include "../../../common/src/misc/lz4.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include "../../../common/nujel/lib/datatypes/closure.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __EMSCRIPTEN__
#include "server_wasm.h"
#elif defined __MINGW32__
#include "server_win.h"
#else
#include "server_bsd.h"
#endif

#ifdef __EMSCRIPTEN__
clientConnection clients[2];
#else
clientConnection clients[32];
#endif
uint clientCount = 0;

const char *getPlayerLeaveMessage(uint c){
	static char msg[256];
	if(c >= 32){return "Someone left";}
	if(*clients[c].playerName < 0x20){return "Someone left";}
	snprintf(msg,sizeof(msg),"%s[%u] left",clients[c].playerName,c);

	return msg;
}

void serverKeepalive(){
	static char buffer[16];
	static u64 lastKA=0;
	u64 ct = getTicks();
	if(ct > lastKA+1000){
		lastKA = ct;
		for(uint i=0;i<clientCount;i++){
			if(clients[i].state){ continue; }
			sendToAll(buffer,16);
		}
	}else if(ct < lastKA){
		lastKA = ct;
	}
}

void serverSendChatMsg(const char *msg){
	packet *p = &packetBuffer;
	size_t len = snprintf((void *)packetBuffer.v.u8,sizeof(packetBuffer.v.u8),"%s",msg);
	packetQueue(p,msgtChatMsg,alignedLen(len+1),-1);
	printf("%s[MSG]%s %s\n",termColors[6],termReset,p->v.u8);
}

void sendPlayerNames(){
	for(uint i=0;i<clientCount;i++){
		if(clients[i].state == STATE_CLOSED){continue;}
		msgPlayerName(-1,i,clients[i].playerName);
	}
}

void sendPlayerJoinMessage(uint c){
	char msg[256];
	sendPlayerNames();
	if((clientCount == 1) && optionSingleplayer){
		snprintf(msg,sizeof(msg),"Started Singleplayer Server [Seed=%i]",optionWorldSeed);
		serverSendChatMsg(msg);
		return;
	}
	snprintf(msg,sizeof(msg),"%s[%u] joined",clients[c].playerName,c);
	serverSendChatMsg(msg);
}

void serverParseChatMsg(uint c,const packet *m){
	(void)c;
	serverSendChatMsg((void *)m->v.u8);
}

void msgPlayerSpawnPos(uint c){
	const vec spawn = worldGetSpawnPos();
	const vec nrot  = vecNew(135.f,15.f,0.f);
	const vec spos  = vecAdd(spawn,vecNew(1.f,4.f,1.f));
	msgPlayerSetPos(c,spos,nrot,vecZero());
	if(clients[c].c != NULL){clients[c].c->pos = spos;}
}

void serverInitClient(uint c, u64 socket){
	memset(&clients[c],0,sizeof(clients[c]));
	clients[c].socket = socket;
	const vec spawn = vecAdd(worldGetSpawnPos(),vecNew(1.f,4.f,1.f));
	clients[c].c                        = characterNew();
	clients[c].cl                       = lispClientClosure(c);
	clients[c].state                    = STATE_CONNECTING;
	clients[c].animalUpdateWindowSize   = 1;
	clients[c].itemDropUpdateWindowSize = 1;
	clients[c].c->pos = spawn;

	bigchungusUnsubscribeClient(&world,c);
}

void addPriorityItemDrop(u16 d){
	for(uint c=0;c<clientCount;c++){
		if(clients[c].state)                          {continue;}
		if(clients[c].itemDropPriorityQueueLen > 127) {continue;}

		for(uint i=0;i<clients[c].itemDropPriorityQueueLen;i++){
			if(clients[c].itemDropPriorityQueue[i] == d){goto continueClientLoop;}
		}
		clients[c].itemDropPriorityQueue[clients[c].itemDropPriorityQueueLen++] = d;
		continueClientLoop:
		(void)d;
	}
}

void delPriorityItemDrop(u16 d){
	for(uint c=0;c<clientCount;c++){
		if(clients[c].state){continue;}

		u16 *iq = clients[c].itemDropPriorityQueue;
		for(uint i=0;i<clients[c].itemDropPriorityQueueLen;i++){
			if(iq[i] != d){continue;}
			iq[i] = iq[--clients[c].itemDropPriorityQueueLen];
		}
	}
}

void msgUpdatePlayer(uint c){
	packet *rp = &packetBuffer;

	for(uint i=0;i<clientCount;i++){
		if(i==c)                {continue;}
		if(clients[i].state)    {continue;}
		if(clients[i].c == NULL){continue;}
		item *itm = characterGetItemBarSlot(clients[i].c,clients[i].c->activeItem);
		const character *chr = clients[i].c;
		int pLen = 16*4;


		rp->v.f[ 0]   = chr->pos.x;
		rp->v.f[ 1]   = chr->pos.y;
		rp->v.f[ 2]   = chr->pos.z;

		rp->v.f[ 3]   = chr->rot.yaw;
		rp->v.f[ 4]   = chr->rot.pitch;
		rp->v.f[ 5]   = chr->yoff;

		rp->v.f[ 6]   = chr->vel.x;
		rp->v.f[ 7]   = chr->vel.y;
		rp->v.f[ 8]   = chr->vel.z;

		rp->v.u32[ 9] = chr->flags;

		rp->v.u16[20] = chr->blockMiningX;
		rp->v.u16[21] = chr->blockMiningY;
		rp->v.u16[22] = chr->blockMiningZ;
		rp->v.u16[23] = chr->hp;

		if(itm == NULL){
			rp->v.u16[24] = 0;
		}else{
			rp->v.u16[24] = itm->ID;
		}
		rp->v.u16[25] = chr->animationIndex;
		rp->v.u16[26] = chr->animationTicksMax;
		rp->v.u16[27] = chr->animationTicksLeft;

		rp->v.u32[15] = i;

		if(chr->hook != NULL){
			rp->v.f[16]   = chr->hookPos.x;
			rp->v.f[17]   = chr->hookPos.y;
			rp->v.f[18]   = chr->hookPos.z;
			pLen = 19*4;
		}

		packetQueue(rp,msgtCharacterUpdate,pLen,c);
	}

	clients[c].itemDropUpdateOffset = itemDropUpdatePlayer(c,clients[c].itemDropUpdateOffset);
	grenadeUpdatePlayer(c);
	blockMiningUpdatePlayer(c);
	animalSyncPlayer(c);
	projectileSyncPlayer(c);
	fireSyncPlayer(c);
	throwableSyncPlayer(c);
	addQueuedChunks(c);
	clients[c].flags &= ~(CONNECTION_DO_UPDATE);
	clients[c].syncCount++;
}

void serverParsePlayerPos(uint c,const packet *p){
	if(clients[c].c == NULL){return;}

	clients[c].c->pos                = vecNewP(&p->v.f[ 0]);
	clients[c].c->rot                = vecNewP(&p->v.f[ 3]);
	clients[c].c->rot.roll           = 0;
	clients[c].c->yoff               = p->v.f[ 5];
	clients[c].c->vel                = vecNewP(&p->v.f[ 6]);

	clients[c].c->flags              = p->v.u32[ 9];

	clients[c].c->blockMiningX       = p->v.u16[20];
	clients[c].c->blockMiningY       = p->v.u16[21];
	clients[c].c->blockMiningZ       = p->v.u16[22];
	clients[c].c->hp                 = p->v.u16[23];

	clients[c].c->activeItem         = p->v.u16[24];
	clients[c].c->animationIndex     = p->v.u16[25];
	clients[c].c->animationTicksMax  = p->v.u16[26];
	clients[c].c->animationTicksLeft = p->v.u16[27];

	if(packetLen(p) >= 18*4 ){
		clients[c].c->hook           = (hook *)0x8;
		clients[c].c->hookPos        = vecNewP(&p->v.f[15]);
	}else{
		clients[c].c->hook           = NULL;
	}

	clients[c].flags |= CONNECTION_DO_UPDATE;
}

void msgSendChunk(uint c, const chunk *chnk){
	packet *p = &packetBuffer;
	memcpy(p->v.u8,chnk->data,sizeof(chnk->data));
	p->v.u16[2048] = chnk->x;
	p->v.u16[2049] = chnk->y;
	p->v.u16[2050] = chnk->z;
	packetQueue(p,msgtChunkData,1026*4,c);
}

void dispatchBeingDmg(uint c, const packet *p){
	(void)c;

	const i16 hp              = p->v.i16[0];
	const u8 cause            = p->v.u8[2];
	const float knockbackMult = ((float)p->v.u8[3])/16.f;
	const being target        = p->v.u32[1];
	const being culprit       = p->v.u32[2];
	const vec pos             = vecNewP(&p->v.f[3]);

	beingDamage(target, hp, cause, knockbackMult, culprit, pos);
}

void handlePingPong(uint c){
	u64 curPing = getTicks();
	clients[c].lastPing = curPing;
}

uint getClientLatency(uint c){
	if(clients[c].lastPing == 0){return 1;}
	return getTicks() - clients[c].lastPing;
}

void packetEcho(u8 c,const packet *p){
	(void)c;
	sendToAll(p,packetLen(p)+4);
}

void packetEchoExcept(u8 c,const packet *p){
	sendToAllExcept(c,p,packetLen(p)+4);
}

void beingMove(u8 c, const packet *p){
	(void)c;
	being b = p->v.u32[0];
	vec dpos = vecNewP(&p->v.f[1]);
	vec dvel = vecNewP(&p->v.f[4]);
	beingAddPos(b,dpos);
	beingAddVel(b,dvel);
}

void serverParseSinglePacket(uint c, packet *p){
	const int pLen  = p->typesize >> 10;
	const int pType = p->typesize & 0xFF;

	if(packetFalseChecksum(p)){
		fprintf(stderr,"[SRV][%i] CHECKSUM WRONG!!! T:%i L:%i\n",c,pType,pLen);
		return;
	}
	nprofAddPacket(pType,pLen);

	switch((messageType)pType){
	case msgtKeepalive:
		break;
	case msgtRequestSpawnPos:
		msgPlayerSpawnPos(c);
		break;
	case msgtRequestChungus:
		addChungusToQueue(c,p->v.u8[0],p->v.u8[1],p->v.u8[2]);
		break;
	case msgtPlaceBlock:
		worldSetB(p->v.u16[0],p->v.u16[1],p->v.u16[2],p->v.u16[3]);
		break;
	case msgtMineBlock:
		blockMiningMineBlock(p->v.u16[0],p->v.u16[1],p->v.u16[2],p->v.u8[7]);
		break;
	case msgtGoodbye:
		errno=0;
		if(!optionSingleplayer){
			msgGoodbye(c);
			serverSendClient(c);
			serverKill(c);
		}else if(clientCount == 1){
			quit = true;
		}
		printf("[SRV] Client said goodbye\n");
		break;
	case msgtItemDropNew:
		itemDropNewPacket(c,p);
		break;
	case msgtGrenadeNew:
		grenadeNewP(p);
		break;
	case msgtBeamblast:
		beamblastNewP(c,p);
		break;
	case msgtCharacterUpdate:
		serverParsePlayerPos(c,p);
		break;
	case msgtChatMsg:
		serverParseChatMsg(c,p);
		break;
	case msgtFxBeamBlaster:
		beamblastNewP(c,p);
		break;
	case msgtBeingDamage:
		dispatchBeingDmg(c,p);
		break;
	case msgtChungusUnsub:
		chungusUnsubscribePlayer(world.chungi[p->v.u8[0]][p->v.u8[1]&0x7F][p->v.u8[2]],c);
		break;
	case msgtCharacterSetInventory:
		characterSetInventoryP(clients[c].c,p);
		break;
	case msgtDirtyChunk:
		worldDirtyChunk(c,p->v.u16[0],p->v.u16[1],p->v.u16[2]);
		break;
	case msgtPingPong:
		handlePingPong(c);
		msgPingPong(c);
		break;
	case msgtCharacterSetEquipment:
		characterSetEquipmentP(clients[c].c,p);
		break;
	case msgtItemDropPickup:
		itemDropPickupP(c,p);
		break;
	case msgtItemDropBounce:
		itemDropBounceP(c,p);
		break;
	case msgtRopeUpdate:
		ropeUpdateP(c,p);
		break;
	case msgtProjectileUpdate:
		projectileRecvUpdate(c,p);
		break;
	case msgtFxProjectileHit:
		packetEchoExcept(c,p);
		break;
	case msgtFireRecvUpdate:
		fireRecvUpdate(c,p);
		break;
	case msgtLispRecvSExpr:
		lispRecvSExpr(c,p);
		break;
	case msgtThrowableRecvUpdates:
		throwableRecvUpdate(p);
		break;
	case msgtBeingMove:
		beingMove(c,p);
		break;

	case msgtLZ4:
	case msgtPlayerPos:
	case msgtChunkData :
	case msgtSetPlayerCount:
	case msgtPlayerPickupItem:
	case msgtExplode:
	case msgtGrenadeUpdate:
	case msgtBlockMiningUpdate:
	case msgtSetChungusLoaded:
	case msgtBeingGotHit:
	case msgtSetTime:
	case msgtPlayerMoveDelta:
	case msgtCharacterName:
	case msgtCharacterSetData:
	case msgtAnimalSync:
	case msgtFxAnimalDied:
	case msgtWeatherRecvUpdate:
	case msgtRainRecvUpdate:
	case msgtItemDropUpdate:
		fprintf(stderr,"%s[%u] received from client, which should never happen\n",networkGetMessageName(pType),pType);
		serverKill(c);
		break;
	}
}

void serverParsePacket(uint i){
	uint off = clients[i].recvBufOff;
	if(i >= clientCount){return;}

	if(clients[i].flags & CONNECTION_WEBSOCKET){
		serverParseWebSocketPacket(i);
	}

	for(int max=64;max > 0;--max){
		if((clients[i].recvBufLen-off) < 4)     {break; }
		uint pLen = packetLen((packet *)(clients[i].recvBuf+off));
		if((pLen+4) > clients[i].recvBufLen-off){break; }
		serverParseSinglePacket(i,(packet *)(clients[i].recvBuf+off));
		off += pLen+4;
		if(clients[i].state){break;}
	}

	if(off > clients[i].recvBufLen){
		fprintf(stderr,"[SRV] Offset greater than buffer length, sumething went horribly wrong...\n");
		serverKill(i);
	} else if(off == clients[i].recvBufLen){
		clients[i].recvBufLen = 0;
		off = 0;
	} else if(off > sizeof(clients[i].recvBuf)/2){
		fprintf(stderr,"[SRV] Couldn't parse all packets, off=%i len=%i\n",off,clients[i].recvBufLen);
		memmove(clients[i].recvBuf,&clients[i].recvBuf[off],(clients[i].recvBufLen - off));
		clients[i].recvBufLen -= off;
		off = 0;
	}
	clients[i].recvBufOff = off;
}

/* TODO: what happens on a HEAD request */
void serverParseConnection(uint c){
	for(uint ii=3;ii<clients[c].recvBufLen;ii++){
		if(clients[c].recvBuf[ii  ] != '\n'){ continue; }
		if(clients[c].recvBuf[ii-1] != '\r'){ continue; }
		if(clients[c].recvBuf[ii-2] != '\n'){ continue; }
		if(clients[c].recvBuf[ii-3] != '\r'){ continue; }

		if( (clients[c].recvBuf[0] == 'G') &&
			(clients[c].recvBuf[1] == 'E') &&
			(clients[c].recvBuf[2] == 'T') &&
			(clients[c].recvBuf[3] == ' ')
		){
			serverParseWebSocketHeader(c,ii-1);
		}
		for(uint i=0;i<clients[c].recvBufLen-(ii);i++){
			clients[c].recvBuf[i] = clients[c].recvBuf[i+(ii+1)];
		}
		clients[c].recvBufLen -= ii+1;
		clients[c].state       = STATE_INTRO;
		return;
	}
}

void serverParseIntro(uint c){
	#ifndef __EMSCRIPTEN__
	if(clients[c].flags & CONNECTION_WEBSOCKET){
		serverParseWebSocketPacket(c);
	}
	#endif

	for(uint ii=0;ii<clients[c].recvBufLen;ii++){
		if(clients[c].recvBuf[ii] != '\n'){ continue; }
		memcpy(clients[c].playerName,clients[c].recvBuf,MIN(sizeof(clients[c].playerName)-1,ii));
		clients[c].playerName[sizeof(clients[c].playerName)-1] = 0;
		for(uint i=0;i<clientCount;i++){
			if(i == c){continue;}
			if(strncmp(clients[c].playerName,clients[i].playerName,sizeof(clients[i].playerName)) != 0){continue;}
			fprintf(stderr,"[SRV] %s already in use\n",clients[c].playerName);
			serverKill(c);
			return;
		}
		for(uint i=0;i<clients[c].recvBufLen-(ii);i++){
			clients[c].recvBuf[i] = clients[c].recvBuf[i+(ii+1)];
		}
		clients[c].recvBufLen -= ii+1;
		clients[c].state = STATE_READY;

		characterLoadSendData(clients[c].c,clients[c].playerName,c);
		sendPlayerJoinMessage(c);
		msgSetTime(c, gtimeGetTime());
		animalUpdatePriorities(c);
		weatherSendUpdate(c);
		clients[c].lastPing = getTicks();
		msgPingPong(c);
	}
}

void serverParse(){
	for(uint i=0;i<clientCount;i++){
		switch(clients[i].state){
		case STATE_READY:
			serverParsePacket(i);
			break;
		case STATE_CONNECTING:
			serverParseConnection(i);
			break;
		case STATE_INTRO:
			serverParseIntro(i);
			break;
		default:
		case STATE_CLOSED:
			break;
		}
	}
}

void addChungusToQueue(uint c, u8 x, u8 y, u8 z){
	if(c >= clientCount){ return; }
	if(clients[c].state){ return; }
	if(clients[c].chngReqQueueLen >= countof(clients[c].chngReqQueue)){
		return;
	}
	for(uint i=0;i<clients[c].chngReqQueueLen;i++){
		chungusReqEntry *e = &clients[c].chngReqQueue[i];
		if(e->x != x){continue;}
		if(e->y != y){continue;}
		if(e->z != z){continue;}
		return;
	}
	clients[c].chngReqQueue[clients[c].chngReqQueueLen++] = (chungusReqEntry){x,y,z,0};
}

void addChunkToQueue(uint c, u16 x, u16 y, u16 z){
	if(c >= clientCount){return;}
	if(clients[c].state){ return; }
	if(clients[c].chnkReqQueueLen >= countof(clients[c].chnkReqQueue)){
		fprintf(stderr,"Chunk Request Queue full!\n");
		return;
	}
	clients[c].chnkReqQueue[clients[c].chnkReqQueueLen++] = (chunkReqEntry){x,y,z,0};
}

void addChunksToQueue(uint c){
	if(c >= clientCount){return;}
	if(clients[c].chngReqQueueLen == 0){return;}
	const chungusReqEntry entry = clients[c].chngReqQueue[--clients[c].chngReqQueueLen];
	chungus *chng = worldGetChungus(entry.x,entry.y,entry.z);
	if(chng == NULL){ return; }
	float dist = chungusDistance(clients[c].c, chng);
	if(dist > 4096.f){
		fprintf(stderr,"Requested Chungus too far away Chungus(%u, %u, %u) Player(%f, %f, %f))\n",chng->x<<8,chng->y<<8,chng->z<<8,clients[c].c->pos.x,clients[c].c->pos.y,clients[c].c->pos.z);
		return;
	}
	clients[c].chnkReqQueue[clients[c].chnkReqQueueLen++] = (chunkReqEntry){entry.x,entry.y,entry.z,0xFF};
	for(int x=15;x>= 0;--x){
	for(int y=15;y>= 0;--y){
	for(int z=15;z>= 0;--z){
		if(chng->chunks[x][y][z] == NULL){continue;}
		addChunkToQueue(c,(entry.x<<8)|(x<<4),(entry.y<<8)|(y<<4),(entry.z<<8)|(z<<4));
	}
	}
	}
	chungusSetUpdated(chng,c);
}

void addQueuedChunks(uint c){
	while(clients[c].sendBufLen < (sizeof(clients[c].sendBuf)-(1<<16))){
		if(clients[c].chnkReqQueueLen == 0){
			if(clients[c].chngReqQueueLen == 0){
				return;
			}
			addChunksToQueue(c);
			return;
		}
		const chunkReqEntry entry = clients[c].chnkReqQueue[--clients[c].chnkReqQueueLen];
		if(entry.w == 0xFF){
			msgSendChungusComplete(c,entry.x,entry.y,entry.z);
		}else{
			chunk *chnk = worldGetChunk(entry.x,entry.y,entry.z);
			if(chnk == NULL)          {continue;}
			msgSendChunk(c,chnk);
			chunkSetUpdated(chnk,c);
		}
	}
}

void serverCheckCompression(int c){
	int len,compressLen;
	u8 *start;
	static u8 compressBuf[LZ4_COMPRESSBOUND(sizeof(clients[c].sendBuf))];
	//if(clients[c].flags & CONNECTION_WEBSOCKET){return;}
	start = &clients[c].sendBuf[clients[c].sendBufLastCompressed];
	len = &clients[c].sendBuf[clients[c].sendBufLen] - start;
	if(len <= (1<<8)){return;}

	compressLen = LZ4_compress_default((const char *)start, (char *)compressBuf, len, sizeof(compressBuf));
	if(compressLen > len){
		fprintf(stderr,"%i > %i = Compression does not decrease size\n",compressLen,len);
		return;
	}
	//printf("Z %u -> %u\n",len,compressLen);
	clients[c].sendBufLen -= len;
	memcpy(start + 4,compressBuf,compressLen);
	packetSet((packet *)start,msgtLZ4,compressLen);
	clients[c].sendBufLen += alignedLen(compressLen + 4);
	clients[c].sendBufLastCompressed += alignedLen(compressLen + 4);
}

void serverSend(){
	for(uint i=0;i<clientCount;i++){
		if(clients[i].state){ continue; }
		if(clients[i].flags & CONNECTION_DO_UPDATE){msgUpdatePlayer(i);}
		if(clients[i].sendBufLen == 0){ continue; }
		addQueuedChunks(i);
		bigchungusUpdateClient(&world,i);
		#ifndef __EMSCRIPTEN__
		serverCheckCompression(i);
		#endif
		serverSendClient(i);
	}
}

void serverHandleEvents(){
	serverAccept();
	serverRead();
	serverParse();
	serverKeepalive();
	ropeSyncAll();
	serverSend();
}

int serverSendClient(uint c){
	const uint len = clients[c].sendBufLen-clients[c].sendBufSent;
	if(len > 0){
		uint ret = serverSendRaw(c,clients[c].sendBuf+clients[c].sendBufSent,len);
		clients[c].sendBufSent += ret;
	}
	if(clients[c].sendBufSent >= clients[c].sendBufLen){
		clients[c].sendBufLastCompressed = 0;
		clients[c].sendBufSent = 0;
		clients[c].sendBufLen  = 0;
		return 0;
	}
	return 1;
}

void sendToClient(uint c,const void *data,uint len){
	int ret;
	int tlen = len;
	if(clients[c].state){ return; }
	if(c >= clientCount){ return; }
	if(clients[c].flags & CONNECTION_WEBSOCKET){
		tlen += 10;
	}

	while(clients[c].sendBufLen+tlen > (int)sizeof(clients[c].sendBuf)){
		serverCheckCompression(c);
		ret = serverSendClient(c);
		if(ret == 1){
			usleep(10);
		}else if(ret == 2){
			return;
		}
	}
	if(clients[c].flags & CONNECTION_WEBSOCKET){
		clients[c].sendBufLen += addWSMessagePrefix(clients[c].sendBuf + clients[c].sendBufLen,len,sizeof(clients[c].sendBuf)-clients[c].sendBufLen);
	}
	memcpy(clients[c].sendBuf+clients[c].sendBufLen,data,len);
	clients[c].sendBufLen += len;
}

void sendToAll(const void *data, uint len){
	for(uint i=0;i<clientCount;i++){
		sendToClient(i,data,len);
	}
}

void sendToAllExcept(uint e,const void *data, uint len){
	for(uint i=0;i<clientCount;i++){
		if(i==e){continue;}
		sendToClient(i,data,len);
	}
}

void serverCloseClient(uint c){
	const char *msg = getPlayerLeaveMessage(c);
	characterSaveData(clients[c].c,clients[c].playerName);
	if(clients[c].c != NULL){
		characterFree(clients[c].c);
		clients[c].c = NULL;
	}
	if(clients[c].state == STATE_READY){
		serverSendChatMsg(msg);
	}
	clients[c].state = STATE_CLOSED;
	msgSetPlayerCount(c,clientCount);
	sendPlayerNames();
	lClosureFree(clients[c].cl - lClosureList);
	clients[c].cl = NULL;

	int lowestClient=0;
	for(uint i=0;i<clientCount;i++){
		if(clients[i].state != STATE_CLOSED){
			lowestClient=i;
		}
	}
	clientCount = lowestClient+1;
	if((clientCount == 1) && (clients[0].state == STATE_CLOSED)){
		clientCount = 0;
	}
	bigchungusSafeSave(&world,true);
}

int getClientByName(const char *name){
	for(uint i=0;i<clientCount;i++){
		if(strncasecmp(clients[i].playerName,name,32) == 0){
			return i;
		}
	}
	return -1;
}

int getClientByCharacter(const character *c){
	if(c == NULL){return -1;}
	for(uint i=0;i<clientCount;i++){
		if(clients[i].c == c){return i;}
	}
	return -1;
}

bool getClientValid(uint c){
	if(c > clientCount)     {return false;}
	if(clients[c].state)    {return false;}
	if(clients[c].c == NULL){return false;}
	return true;
}
