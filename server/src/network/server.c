#define _DEFAULT_SOURCE
#include "server.h"

#include "../main.h"
#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../game/grenade.h"
#include "../misc/options.h"
#include "../network/server_ws.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/lz4.h"
#include "../../../common/src/misc.h"
#include "../../../common/src/messages.h"
#include "../../../common/src/packet.h"

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
int clientCount = 0;

char *getPlayerLeaveMessage(int c){
	static char msg[256];
	snprintf(msg,sizeof(msg),"%s left",clients[c].playerName);
	return msg;
}

void serverKeepalive(){
	static char buffer[16];
	static uint64_t lastKA=0;
	uint64_t ct = getMillis();
	if(ct > lastKA+1000){
		lastKA = ct;
		for(int i=0;i<clientCount;i++){
			sendToAll(buffer,16);
		}
	}else if(ct < lastKA){
		lastKA = ct;
	}
}

void serverSendChatMsg(const char *msg){
	packet *p = (packet *)packetBuffer;
	strncpy((char *)(p->val.c+2),msg,254);
	p->val.c[255] = 0;
	packetQueue(p,16,256,-1);
	printf("%s[MSG]%s %s\n",termColors[6],termReset,msg);
}

void sendPlayerJoinMessage(int c){
	char msg[256];
	if((clientCount == 1) && optionSingleplayer){
		snprintf(msg,sizeof(msg),"Started Singleplayer Server [Seed=%i]",optionWorldSeed);
		serverSendChatMsg(msg);
		return;
	}
	snprintf(msg,sizeof(msg),"%s joined",clients[c].playerName);
	serverSendChatMsg(msg);
}

void serverParseChatMsg(int c, packet *m){
	char msg[256];
	snprintf(msg,sizeof(msg),"%s: %s",clients[c].playerName,(char *)(m->val.c+2));
	serverSendChatMsg(msg);
}

void serverParseDyingMsg(int c, packet *m){
	char msg[256];
	if((m->val.s[0] != 65535) && (m->val.s[0] < clientCount)){
		snprintf(msg,sizeof(msg),"%s %s %s",clients[m->val.s[0]].playerName,(char *)(m->val.c+2),clients[c].playerName);
	}else{
		snprintf(msg,sizeof(msg),"%s %s",clients[c].playerName,(char *)(m->val.c+2));
	}
	serverSendChatMsg(msg);
}

void msgPlayerSpawnPos(int c){
	packet *p = (packet *)packetBuffer;
	int sx,sy,sz;
	worldGetSpawnPos(&sx,&sy,&sz);
	p->val.f[0] = ((float)sx)+0.5f;
	p->val.f[1] = ((float)sy)+2.0f;
	p->val.f[2] = ((float)sz)+0.5f;
	packetQueue(p,1,3*4,c);
}

void serverIntro(int c){
	itemDropIntro(c);
}

void msgUpdatePlayer(int c){
	packet *rp = (packet *)packetBuffer;

	for(int i=0;i<clientCount;++i){
		if(i==c)                {continue;}
		if(clients[i].c == NULL){continue;}

		rp->val.f[ 0] = clients[i].c->x;
		rp->val.f[ 1] = clients[i].c->y;
		rp->val.f[ 2] = clients[i].c->z;
		rp->val.f[ 3] = clients[i].c->yaw;
		rp->val.f[ 4] = clients[i].c->pitch;
		rp->val.f[ 5] = clients[i].c->roll;
		rp->val.f[ 6] = clients[i].c->vx;
		rp->val.f[ 7] = clients[i].c->vy;
		rp->val.f[ 8] = clients[i].c->vz;
		rp->val.f[ 9] = clients[i].c->yoff;
		rp->val.i[10] = clients[i].c->hook;
		rp->val.f[11] = clients[i].c->hookx;
		rp->val.f[12] = clients[i].c->hooky;
		rp->val.f[13] = clients[i].c->hookz;
		rp->val.i[14] = clients[i].c->blockMiningX;
		rp->val.i[15] = clients[i].c->blockMiningY;
		rp->val.i[16] = clients[i].c->blockMiningZ;
		rp->val.i[17] = clients[i].c->activeItem;
		rp->val.i[18] = clients[i].c->hitOff;
		rp->val.i[19] = i;
		packetQueue(rp,15,20*4,c);
	}

	clients[c].itemDropUpdateOffset = itemDropUpdatePlayer(c,clients[c].itemDropUpdateOffset);
	grenadeUpdatePlayer(c);
	blockMiningUpdatePlayer(c);
}

void serverParsePlayerPos(int c, packet *p){
	clients[c].c->x            = p->val.f[ 0];
	clients[c].c->y            = p->val.f[ 1];
	clients[c].c->z            = p->val.f[ 2];
	clients[c].c->yaw          = p->val.f[ 3];
	clients[c].c->pitch        = p->val.f[ 4];
	clients[c].c->roll         = p->val.f[ 5];
	clients[c].c->vx           = p->val.f[ 6];
	clients[c].c->vy           = p->val.f[ 7];
	clients[c].c->vz           = p->val.f[ 8];
	clients[c].c->yoff         = p->val.f[ 9];
	clients[c].c->hook         = p->val.i[10];
	clients[c].c->hookx        = p->val.f[11];
	clients[c].c->hooky        = p->val.f[12];
	clients[c].c->hookz        = p->val.f[13];
	clients[c].c->blockMiningX = p->val.i[14];
	clients[c].c->blockMiningY = p->val.i[15];
	clients[c].c->blockMiningZ = p->val.i[16];
	clients[c].c->activeItem   = p->val.i[17];
	clients[c].c->hitOff       = p->val.i[18];
	msgUpdatePlayer(c);
}

void msgSendChunk(int c, const chunk *chnk){
	packet *p = (packet *)packetBuffer;
	memcpy(p->val.c,chnk->data,sizeof(chnk->data));
	p->val.i[1024] = chnk->x;
	p->val.i[1025] = chnk->y;
	p->val.i[1026] = chnk->z;
	packetQueue(p,18,1027*4,c);
}

void serverParseSinglePacket(int c, packet *p){
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
			addChungusToQueue(c,p->val.i[0],p->val.i[1],p->val.i[2]);
			if(verbose){printf("[%02i] requestChungus\n",c);}
		break;

		case 3: // placeBlock
			worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],p->val.i[3]);
			sendToAllExcept(c,p,pLen+4);
			if(verbose){printf("[%02i] placeBlock\n",c);}
		break;

		case 4: // mineBlock
			blockMiningDropItemsPos(p->val.i[0],p->val.i[1],p->val.i[2],worldGetB(p->val.i[0],p->val.i[1],p->val.i[2]));
			worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],0);
			sendToAllExcept(c,p,pLen+4);
			if(verbose){printf("[%02i] mineBlock\n",c);}
		break;

		case 5: // Goodbye
			errno=0;
			serverKill(c);
		break;

		case 6: // blockMiningUpdate
			fprintf(stderr,"blockMiningUpdate received from client, which should never happen\n");
			serverKill(c);
		break;

		case 7:
			fprintf(stderr,"worldSetChungusLoaded received from client, which should never happen\n");
			serverKill(c);
		break;

		case 8: // CharacterGotHit
			msgCharacterGotHit(c,p->val.f[0]);
			if(verbose){printf("[%02i] msgCharacterGotHit\n",c);}
		break;

		case 9: // PlayerJoin
			memcpy(clients[c].playerName,p->val.c,28);
			clients[c].playerName[sizeof(clients[c].playerName)-1] = 0;
			sendPlayerJoinMessage(c);
			if(verbose){printf("[%02i] sendPlayerName\n",c);}
		break;

		case 10: // itemDropNew
			itemDropNewC(p);
			if(verbose){printf("[%02i] itemDropNewC\n",c);}
		break;

		case 11:
			grenadeNew(p);
			if(verbose){printf("[%02i] grenadeNew\n",c);}
		break;

		case 12:
			beamblast(c,p);
			if(verbose){printf("[%02i] beamblast\n",c);}
		break;

		case 13:
			fprintf(stderr,"playerMoveDelta received from client, which should never happen\n");
			serverKill(c);
		break;

		case 14:
			msgCharacterHit(c,p->val.f[0],p->val.f[1],p->val.f[2],p->val.f[3],p->val.f[4],p->val.f[5],p->val.f[6]);
			if(verbose){printf("[%02i] characterHit\n",c);}
		break;

		case 15:
			serverParsePlayerPos(c,p);
			if(verbose){printf("[%02i] sendPlayerPos\n",c);}
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
			fprintf(stderr,"fxBeamBlaster received from client, which should never happen\n");
			serverKill(c);
		break;

		default:
			printf("[%i] %i[%i] UNKNOWN PACKET\n",c,pType,pLen);
			serverKill(c);
		break;
	}
}

void serverParsePacket(int i){
	unsigned int off = 0;

	for(int max=32;max > 0;--max){
		if((i < 0) || (i >= clientCount)){break;}
		if((clients[i].recvBufLen-off) < 4){ break; }
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
	} else {
		for(unsigned int ii=0;ii < (clients[i].recvBufLen - off);++ii){
			clients[i].recvBuf[ii] = clients[i].recvBuf[ii+off];
		}
		clients[i].recvBufLen -= off;
	}
}

void serverParseWebSocket(int c,int end){
	clients[c].recvBuf[end-2] = 0;
	if((clients[c].recvBuf[0] == 'G') && (clients[c].recvBuf[1] == 'E') && (clients[c].recvBuf[2] == 'T') && (clients[c].recvBuf[3] == ' ')){
		serverParseWebSocketHeader(c,end);
	}
	for(unsigned int i=0;i<clients[c].recvBufLen-end;i++){
		clients[c].recvBuf[i] = clients[c].recvBuf[i+end];
	}
	clients[c].recvBufLen -= end;
	clients[c].state = 1;
	serverIntro(c);
	serverParsePacket(c);
}

void serverParseIntroduction(int i){
	if(clients[i].recvBufLen < 4){ return; }
	for(unsigned int ii=3;ii<clients[i].recvBufLen;ii++){
		if(clients[i].recvBuf[ii  ] != '\n'){ continue; }
		if(clients[i].recvBuf[ii-1] != '\r'){ continue; }
		if(clients[i].recvBuf[ii-2] != '\n'){ continue; }
		if(clients[i].recvBuf[ii-3] != '\r'){ continue; }
		serverParseWebSocket(i,ii+1);
		break;
	}
}

void serverParse(){
	for(int i=0;i<clientCount;i++){
		switch(clients[i].state){
			case 0:
				serverParseIntroduction(i);
				break;
			default:
			case 1:
				if(clients[i].flags & CONNECTION_WEBSOCKET){
					serverParseWebSocketPacket(i);
				}
				serverParsePacket(i);
				break;
		}
	}
}

void addChungusToQueue(int c, uint16_t x, uint16_t y, uint16_t z){
	uint64_t entry = 0;
	if((c < 0) || (c >= clientCount)){return;}
	if(clients[c].chngReqQueueLen >= sizeof(clients[c].chngReqQueue)){
		printf("Chungus Request Queue full!\n");
		return;
	}
	entry |=  (uint64_t)x & 0xFFFF;
	entry |= ((uint64_t)y & 0xFFFF) << 16;
	entry |= ((uint64_t)z & 0xFFFF) << 32;
	clients[c].chngReqQueue[clients[c].chngReqQueueLen++] = entry;
}

void addChunkToQueue(int c, uint16_t x, uint16_t y, uint16_t z){
	uint64_t entry = 0;
	if((c < 0) || (c >= clientCount)){return;}
	if(clients[c].chnkReqQueueLen >= sizeof(clients[c].chnkReqQueue)){
		return;
	}
	entry |=  (uint64_t)x & 0xFFFF;
	entry |= ((uint64_t)y & 0xFFFF) << 16;
	entry |= ((uint64_t)z & 0xFFFF) << 32;
	clients[c].chnkReqQueue[clients[c].chnkReqQueueLen++] = entry;
}

void addChunksToQueue(int c){
	if((c < 0) || (c >= clientCount)){return;}
	if(clients[c].chngReqQueueLen == 0){return;}
	uint64_t entry = clients[c].chngReqQueue[--clients[c].chngReqQueueLen];
	uint16_t cx =  entry        & 0xFFFF;
	uint16_t cy = (entry >> 16) & 0xFFFF;
	uint16_t cz = (entry >> 32) & 0xFFFF;

	chungus *chng = worldGetChungus(cx>>8,cy>>8,cz>>8);
	if(chng == NULL){return;}
	for(int x=15;x>= 0;--x){
		for(int y=15;y>= 0;--y){
			for(int z=15;z>= 0;--z){
				if(chng->chunks[x][y][z] != NULL){
					addChunkToQueue(c,cx|(x<<4),cy|(y<<4),cz|(z<<4));
				}
			}
		}
	}
	entry |= (uint64_t)1<<62;
	clients[c].chnkReqQueue[clients[c].chnkReqQueueLen++] = entry;
}

void addQueuedChunks(int c){
	while(clients[c].sendBufLen < (sizeof(clients[c].sendBuf)-(1<<16))){
		if(clients[c].chnkReqQueueLen == 0){
			if(clients[c].chngReqQueueLen == 0){
				return;
			}
			addChunksToQueue(c);
			continue;
		}
		uint64_t entry = clients[c].chnkReqQueue[--clients[c].chnkReqQueueLen];
		uint16_t cx =  entry        & 0xFFFF;
		uint16_t cy = (entry >> 16) & 0xFFFF;
		uint16_t cz = (entry >> 32) & 0xFFFF;
		if(entry & ((uint64_t)1<<62)){
			msgSendChungusComplete(c,cx,cy,cz);
		}else{
			chunk *chnk = worldGetChunk(cx,cy,cz);
			if(chnk != NULL){
				msgSendChunk(c,chnk);
			}
		}
	}
}

void *serverFindCompressibleStart(int c, int *len){
	uint8_t *t = NULL;
	uint8_t *ret = NULL;
	*len = 0;
	for(t=clients[c].sendBuf;(t-clients[c].sendBuf) < clients[c].sendBufLen;t+=4+alignedLen(packetLen((packet *)t))){
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
	uint8_t *start;
	static uint8_t compressBuf[LZ4_COMPRESSBOUND(sizeof(clients[c].sendBuf))];
	if(clients[c].flags & CONNECTION_WEBSOCKET){return;}
	if(clients[c].sendBufLen < (int)(sizeof(clients[c].sendBuf)/32)){return;}
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
	clients[c].sendBufLen += alignedLen(compressLen)+4;
}

void serverSend(){
	for(int i=0;i<clientCount;i++){
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

void sendToClient(int c,void *data,int len){
	int ret;
	int tlen = len;
	if(c < 0){return;}
	if(c >= clientCount){return;}
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

void sendToAll(void *data, int len){
	for(int i=0;i<clientCount;i++){
		sendToClient(i,data,len);
	}
}

void sendToAllExcept(int e,void *data, int len){
	for(int i=0;i<clientCount;i++){
		if(i==e){continue;}
		sendToClient(i,data,len);
	}
}

void serverCloseClient(int c){
	char *msg = getPlayerLeaveMessage(c);
	if(clients[c].c != NULL){
		characterFree(clients[c].c);
		clients[c].c = NULL;
	}
	clients[c] = clients[--clientCount];
	msgSetPlayerCount(c,clientCount);
	serverSendChatMsg(msg);
	if((clientCount == 0) && (optionSingleplayer)){
		quit = true;
	}
}
