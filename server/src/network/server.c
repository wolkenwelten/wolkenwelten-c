#include "server.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/itemDrop.h"
#include "../game/grenade.h"
#include "../misc/command.h"
#include "../misc/options.h"
#include "../network/server_ws.h"
#include "../persistence/savegame.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/misc/lz4.h"
#include "../../../common/src/network/messages.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __MINGW32__
#include "server_win.h"
#else
#include "server_bsd.h"
#endif

clientConnection clients[32];
uint clientCount = 0;

const char *getPlayerLeaveMessage(uint c){
	static char msg[256];
	snprintf(msg,sizeof(msg),"%s left",clients[c].playerName);
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
	strncpy((char *)(&p->v.u8[2]),msg,254);
	p->v.u8[255] = 0;
	packetQueue(p,16,256,-1);
	printf("%s[MSG]%s %s\n",termColors[6],termReset,msg);
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
	snprintf(msg,sizeof(msg),"%s joined",clients[c].playerName);
	serverSendChatMsg(msg);
}

void serverParseChatMsg(uint c,const packet *m){
	char msg[256];
	if(parseCommand(c,(const char *)(&m->v.u8[2]))){return;}
	snprintf(msg,sizeof(msg),"%.32s: %.192s",clients[c].playerName,(char *)(&m->v.u8[2]));
	serverSendChatMsg(msg);
}

void serverParseDyingMsg(uint c,const packet *m){
	char msg[256];
	if((m->v.u16[0] != 65535) && (m->v.u16[0] < clientCount)){
		snprintf(msg,sizeof(msg),"%.32s %.128s %.32s",clients[m->v.u16[0]].playerName,(char *)(&m->v.u8[2]),clients[c].playerName);
	}else{
		snprintf(msg,sizeof(msg),"%.32s %.128s",clients[c].playerName,(char *)(&m->v.u8[2]));
	}
	serverSendChatMsg(msg);
}

void msgPlayerSpawnPos(uint c){
	const vec spawn = vecNewI(worldGetSpawnPos());
	const vec nrot  = vecNew(135.f,15.f,0.f);
	msgPlayerSetPos(c,vecAdd(spawn,vecNew(.5f,2.f,.5f)),nrot);
}

void serverInitClient(uint c){
	clients[c].c                        = characterNew();
	clients[c].recvBufLen               = 0;
	clients[c].sendBufSent              = 0;
	clients[c].sendBufLen               = 0;
	clients[c].chngReqQueueLen          = 0;
	clients[c].chnkReqQueueLen          = 0;
	clients[c].state                    = STATE_CONNECTING;
	clients[c].flags                    = 0;
	clients[c].animalUpdateWindowSize   = 1;
	clients[c].animalUpdateOffset       = 0;
	clients[c].animalPriorityQueueLen   = 0;
	clients[c].itemDropUpdateWindowSize = 1;
	clients[c].itemDropUpdateOffset     = 0;
	clients[c].itemDropPriorityQueueLen = 0;

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

void addPriorityAnimal(u16 d){
	for(uint c=0;c<clientCount;c++){
		if(clients[c].state)                        {continue;}
		if(clients[c].animalPriorityQueueLen > 127) {continue;}

		for(uint i=0;i<clients[c].animalPriorityQueueLen;i++){
			if(clients[c].animalPriorityQueue[i] == d){goto continueClientLoop;}
		}
		clients[c].animalPriorityQueue[clients[c].animalPriorityQueueLen++] = d;
		continueClientLoop:
		(void)d;
	}
}

void msgUpdatePlayer(uint c){
	packet *rp = &packetBuffer;

	for(uint i=0;i<clientCount;++i){
		if(i==c)                {continue;}
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

		packetQueue(rp,15,pLen,c);
	}

	clients[c].itemDropUpdateOffset = itemDropUpdatePlayer(c,clients[c].itemDropUpdateOffset);
	grenadeUpdatePlayer(c);
	blockMiningUpdatePlayer(c);
	bigchungusUpdateClient(&world,c);
	clients[c].animalUpdateOffset = animalSyncPlayer(c,clients[c].animalUpdateOffset);
	addQueuedChunks(c);
	clients[c].flags &= ~(CONNECTION_DO_UPDATE);
}

void serverParsePlayerPos(uint c,const packet *p){
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
	packetQueue(p,18,1026*4,c);
}

void dispatchBeingDmg(uint c, const packet *p){
	const being target  = p->v.u32[1];

	switch(beingType(target)){
	default:
		fprintf(stderr,"dispatchBeingDmg[%i]: Unknown being %x",c,target);
		break;
	case BEING_CHARACTER:
		characterDmgPacket(c,p);
		break;
	case BEING_ANIMAL:
		animalDmgPacket(c,p);
		break;
	}
}

void handlePingPong(uint c){
	u64 curPing = getTicks();
	clients[c].lastPing = curPing;
}

uint getClientLatency(uint c){
	if(clients[c].lastPing == 0){return 1;}
	return getTicks() - clients[c].lastPing;
}

void serverParseSinglePacket(uint c, packet *p){
	const int pLen  = p->typesize >> 10;
	const int pType = p->typesize & 0xFF;

	switch(pType){
		case 0: // Keepalive
			if(verbose){printf("[%02i] keepalive %i:%i\n",c,pType,pLen);}
			break;
		case 1: // requestPlayerSpawnPos
			msgPlayerSpawnPos(c);
			if(verbose){printf("[%02i] requestPlayerSpawnPos\n",c);}
			break;
		case 2: // requestChungus
			addChungusToQueue(c,p->v.u8[0],p->v.u8[1],p->v.u8[2]);
			if(verbose){printf("[%02i] requestChungus\n",c);}
			break;
		case 3: // placeBlock
			worldSetB(p->v.u16[0],p->v.u16[1],p->v.u16[2],p->v.u16[3]);
			if(verbose){printf("[%02i] placeBlock\n",c);}
			break;
		case 4: // mineBlock
			blockMiningDropItemsPos(p->v.u16[0],p->v.u16[1],p->v.u16[2],worldGetB(p->v.u16[0],p->v.u16[1],p->v.u16[2]));
			worldSetB(p->v.u16[0],p->v.u16[1],p->v.u16[2],0);
			//sendToAllExcept(c,p,pLen+4);
			if(verbose){printf("[%02i] mineBlock\n",c);}
			break;
		case 5: // Goodbye
			errno=0;
			serverKill(c);
			if(verbose){printf("[%02i] Goodbye\n",c);}
			break;
		case 6: // blockMiningUpdate
			fprintf(stderr,"blockMiningUpdate received from client, which should never happen\n");
			serverKill(c);
			break;
		case 7:
			fprintf(stderr,"worldSetChungusLoaded received from client, which should never happen\n");
			serverKill(c);
			break;
		case 8: // BeingGotHit
			fprintf(stderr,"beingGotHit received from client, which should never happen\n");
			serverKill(c);
			break;
		case 9: // PlayerJoin !!!!1 Still needed???
			fprintf(stderr,"PlayerJoin received from client, which should never happen\n");
			serverKill(c);
			break;
		case 10: // itemDropNew
			itemDropNewC(c,p);
			if(verbose){printf("[%02i][%i] itemDropNewC\n",c,pLen);}
			break;
		case 11:
			grenadeNewP(p);
			if(verbose){printf("[%02i] grenadeNew\n",c);}
			break;
		case 12:
			beamblastNewP(c,p);
			if(verbose){printf("[%02i] beamblast\n",c);}
			break;
		case 13:
			fprintf(stderr,"playerMoveDelta received from client, which should never happen\n");
			serverKill(c);
			break;
		case 14:
			if(verbose){printf("[%02i] characterName\n",c);}
			break;
		case 15:
			serverParsePlayerPos(c,p);
			//if(verbose){printf("[%02i] sendPlayerPos\n",c);}
			break;
		case 16:
			serverParseChatMsg(c,p);
			if(verbose){printf("[%02i] sendChatMsg\n",c);}
			break;
		case 17:
			serverParseDyingMsg(c,p);
			if(verbose){printf("[%02i] sendDyingMsg\n",c);}
			break;
		case 18:
			fprintf(stderr,"chunkData received from client, which should never happen\n");
			serverKill(c);
			break;
		case 19:
			fprintf(stderr,"setPlayerCount received from client, which should never happen\n");
			serverKill(c);
			break;
		case 20:
			fprintf(stderr,"playerPickupItem received from client, which should never happen\n");
			serverKill(c);
			break;
		case 21:
			fprintf(stderr,"itemDropUpdate received from client, which should never happen\n");
			serverKill(c);
			break;
		case 22:
			fprintf(stderr,"grenadeExplode received from client, which should never happen\n");
			serverKill(c);
			break;
		case 23:
			fprintf(stderr,"grenadeUpdate received from client, which should never happen\n");
			serverKill(c);
			break;
		case 24:
			beamblastNewP(c,p);
			if(verbose){printf("[%02i] beamBlaster\n",c);}
			break;
		case 25:
			fprintf(stderr,"msgItemDropUpdate received from client, which should never happen\n");
			serverKill(c);
			break;
		case 26:
			dispatchBeingDmg(c,p);
			break;
		case 27:
			chungusUnsubscribePlayer(world.chungi[p->v.u8[0]][p->v.u8[1]&0x7F][p->v.u8[2]],c);
			break;
		case 28:
			fprintf(stderr,"characterSetData received from client, which should never happen\n");
			serverKill(c);
			break;
		case 29:
			characterSetInventoryP(clients[c].c,p);
			break;
		case 30:
			fprintf(stderr,"animalSync received from client, which should never happen\n");
			serverKill(c);
			break;
		case 31:
			worldDirtyChunk(c,p->v.u16[0],p->v.u16[1],p->v.u16[2]);
			break;
		case 32:
			fprintf(stderr,"animalDmg received from client, which should never happen\n");
			//animalDmgPacket(c,p);
			break;
		case 33:
			handlePingPong(c);
			msgPingPong(c);
			break;
		case 34:
			//fprintf(stderr,"animalDied received from client\n");
			//serverKill(c);
			break;
		case 35:
			characterSetEquipmentP(clients[c].c,p);
			break;
		default:
			printf("[%i] %i[%i] UNKNOWN PACKET\n",c,pType,pLen);
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

	for(int max=32;max > 0;--max){
		if((clients[i].recvBufLen-off) < 4)     { break; }
		unsigned int pLen = packetLen((packet *)(clients[i].recvBuf+off));
		if((pLen+4) > clients[i].recvBufLen-off){ break; }
		serverParseSinglePacket(i,(packet *)(clients[i].recvBuf+off));
		off += pLen+4;
	}

	if(off > clients[i].recvBufLen){
		fprintf(stderr,"Offset greater than buffer length, sumething went horribly wrong...\n");
		serverKill(i);
	} else if(off == clients[i].recvBufLen){
		clients[i].recvBufLen = 0;
		off = 0;
	} else if(off > sizeof(clients[i].recvBuf)/2){
		fprintf(stderr,"Couldn't parse all packets, off=%i len=%i\n",off,clients[i].recvBufLen);
		for(unsigned int ii=0;ii < (clients[i].recvBufLen - off);++ii){
			clients[i].recvBuf[ii] = clients[i].recvBuf[ii+off];
		}
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
	if(clients[c].flags & CONNECTION_WEBSOCKET){
		serverParseWebSocketPacket(c);
	}

	for(uint ii=0;ii<clients[c].recvBufLen;ii++){
		if(clients[c].recvBuf[ii] != '\n'){ continue; }
		memcpy(clients[c].playerName,clients[c].recvBuf,MIN(sizeof(clients[c].playerName)-1,ii));
		clients[c].playerName[sizeof(clients[c].playerName)-1] = 0;
		for(uint i=0;i<clients[c].recvBufLen-(ii);i++){
			clients[c].recvBuf[i] = clients[c].recvBuf[i+(ii+1)];
		}
		clients[c].recvBufLen -= ii+1;
		clients[c].state = STATE_READY;

		characterLoadSendData(clients[c].c,clients[c].playerName,c);

		sendPlayerJoinMessage(c);
		itemDropIntro(c);
		animalIntro(c);
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
	u32 entry = 0;
	if(c >= clientCount){return;}
	if(clients[c].chngReqQueueLen >= sizeof(clients[c].chngReqQueue)){
		printf("Chungus Request Queue full!\n");
		return;
	}
	entry |=  (u32)x & 0xFFF;
	entry |= ((u32)y & 0xFF) <<  8;
	entry |= ((u32)z & 0xFF) << 16;
	clients[c].chngReqQueue[clients[c].chngReqQueueLen++] = entry;
}

void addChunkToQueue(uint c, u16 x, u16 y, u16 z){
	u64 entry = 0;
	if(c >= clientCount){return;}
	if(clients[c].chnkReqQueueLen >= sizeof(clients[c].chnkReqQueue)){
		return;
	}
	entry |=  (u64)x & 0xFFFF;
	entry |= ((u64)y & 0xFFFF) << 16;
	entry |= ((u64)z & 0xFFFF) << 32;
	clients[c].chnkReqQueue[clients[c].chnkReqQueueLen++] = entry;
}

void addChunksToQueue(uint c){
	if(c >= clientCount){return;}
	if(clients[c].chngReqQueueLen == 0){return;}
	u32 entry = clients[c].chngReqQueue[--clients[c].chngReqQueueLen];
	u8 cx =  entry        & 0xFF;
	u8 cy = (entry >>  8) & 0xFF;
	u8 cz = (entry >> 16) & 0xFF;

	chungus *chng = worldGetChungus(cx,cy,cz);
	if(chng == NULL){
		return;
	}
	chungusSubscribePlayer(chng,c);
	for(int x=15;x>= 0;--x){
		for(int y=15;y>= 0;--y){
			for(int z=15;z>= 0;--z){
				if(chng->chunks[x][y][z] != NULL){
					addChunkToQueue(c,(cx<<8)|(x<<4),(cy<<8)|(y<<4),(cz<<8)|(z<<4));
				}
			}
		}
	}
	u64 centry = ((u64)cx<<8) | ((u64)cy<<24) | ((u64)cz<<40);
	clients[c].chnkReqQueue[clients[c].chnkReqQueueLen++] = centry | (u64)1<<62;
	chungusSetUpdated(chng,c);
}

void addQueuedChunks(uint c){
	while(clients[c].sendBufLen < (sizeof(clients[c].sendBuf)-(1<<16))){
		if(clients[c].chnkReqQueueLen == 0){
			if(clients[c].chngReqQueueLen == 0){
				return;
			}
			addChunksToQueue(c);
			continue;
		}
		u64 entry = clients[c].chnkReqQueue[--clients[c].chnkReqQueueLen];
		u16 cx =  entry        & 0xFFFF;
		u16 cy = (entry >> 16) & 0xFFFF;
		u16 cz = (entry >> 32) & 0xFFFF;
		if(entry & ((u64)1<<62)){
			msgSendChungusComplete(c,cx>>8,cy>>8,cz>>8);
		}else{
			chunk *chnk = worldGetChunk(cx,cy,cz);
			if(chnk != NULL){
				if(!chunkIsUpdated(chnk,c)){
					msgSendChunk(c,chnk);
					chunkSetUpdated(chnk,c);
				}
			}
		}
	}
}

void *serverFindCompressibleStart(uint c, int *len){
	u8 *t = NULL;
	u8 *ret = NULL;
	*len = 0;
	for(t=clients[c].sendBuf;(t-clients[c].sendBuf) < (int)clients[c].sendBufLen;t+=alignedLen(4+packetLen((packet *)t))){
		if(packetType((packet *)t) != 0xFF){
			ret = t;
			break;
		}
	}
	if(ret == NULL){return NULL;}
	*len = clients[c].sendBufLen - (t-clients[c].sendBuf);
	return ret;
}

void serverCheckCompression(int c){
	int len,compressLen;
	u8 *start;
	static u8 compressBuf[LZ4_COMPRESSBOUND(sizeof(clients[c].sendBuf))];
	if(clients[c].flags & CONNECTION_WEBSOCKET){return;}
	//if(clients[c].sendBufLen < (int)(sizeof(clients[c].sendBuf)/128)){return;}
	start = serverFindCompressibleStart(c,&len);
	if(len <= (1<<16)){return;}

	compressLen = LZ4_compress_default((const char *)start, (char *)compressBuf, len, sizeof(compressBuf));
	if(compressLen > len){
		fprintf(stderr,"%i > %i = Compression does not decrease size\n",compressLen,len);
		return;
	}
	clients[c].sendBufLen -= len;
	memcpy(start + 4,compressBuf,compressLen);
	packetSet((packet *)start,0xFF,compressLen);
	clients[c].sendBufLen += alignedLen(compressLen + 4);
}

void serverSend(){
	for(uint i=0;i<clientCount;i++){
		if(clients[i].state){ continue; }
		if(clients[i].flags & CONNECTION_DO_UPDATE){msgUpdatePlayer(i);}
		if(clients[i].sendBufLen == 0){ continue; }
		serverCheckCompression(i);
		serverSendClient(i);
		addQueuedChunks(i);
	}
}

void serverHandleEvents(){
	serverAccept();
	serverRead();
	serverParse();
	serverKeepalive();
	serverSend();
}

int serverSendClient(uint c){
	const uint len = clients[c].sendBufLen-clients[c].sendBufSent;
	if(len > 0){
		uint ret = serverSendRaw(c,clients[c].sendBuf+clients[c].sendBufSent,len);
		clients[c].sendBufSent += ret;
	}
	if(clients[c].sendBufSent >= clients[c].sendBufLen){
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

	serverCheckCompression(c);
	while(clients[c].sendBufLen+tlen > (int)sizeof(clients[c].sendBuf)){
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
	clients[c].state = STATE_CLOSED;
	msgSetPlayerCount(c,clientCount);
	serverSendChatMsg(msg);
	sendPlayerNames();

	int lowestClient=0;
	for(uint i=0;i<clientCount;i++){
		if(clients[i].state != STATE_CLOSED){
			lowestClient=i;
		}
	}
	clientCount = lowestClient+1;

	if((clientCount == 1) && (clients[0].state == STATE_CLOSED) && (optionSingleplayer)){
		quit = true;
	}
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
