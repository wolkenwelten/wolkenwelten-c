#define _DEFAULT_SOURCE

#include "client.h"
#include "../main.h"
#include "../gui/menu.h"
#include "../gfx/effects.h"
#include "../game/blockMining.h"
#include "../game/entity.h"
#include "../game/grenade.h"
#include "../game/grapplingHook.h"
#include "../game/itemDrop.h"
#include "../misc/options.h"
#include "../network/chat.h"
#include "../../../common/src/lz4.h"
#include "../../../common/src/misc.h"
#include "../../../common/src/messages.h"
#include "../../../common/src/packet.h"
#include "../voxel/bigchungus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>


int recvBufLen = 0;
uint8_t recvBuf[1<<20];

int sendBufSent = 0;
int sendBufLen  = 0;
uint8_t sendBuf[1<<16];

size_t sentBytesCurrentSession = 0;
size_t recvBytesCurrentSession = 0;

int serverPort        = 6309;
pid_t singlePlayerPID = 0;
int connectionTries   = 0;

#ifdef __MINGW32__
#include "client_win.h"
#else
#include "client_bsd.h"
#endif

void msgParseGetChunk(packet *p){
	int x = p->val.i[1024];
	int y = p->val.i[1025];
	int z = p->val.i[1026];
	chungus *chng =  worldGetChungus(x>>8,y>>8,z>>8);
	if(chng == NULL){return;}
	chunk *chnk = chungusGetChunkOrNew(chng,x,y,z);
	if(chnk == NULL){return;}
	memcpy(chnk->data,p->val.c,sizeof(chnk->data));
	chnk->ready &= ~15;
}

void msgSendPlayerPos(){
	packet *p = alloca(4+19*4);
	item *itm = characterGetItemBarSlot(player,player->activeItem);

	p->val.f[ 0] = player->x;
	p->val.f[ 1] = player->y;
	p->val.f[ 2] = player->z;
	p->val.f[ 3] = player->yaw;
	p->val.f[ 4] = player->pitch;
	p->val.f[ 5] = player->roll;
	p->val.f[ 6] = player->vx;
	p->val.f[ 7] = player->vy;
	p->val.f[ 8] = player->vz;
	p->val.f[ 9] = player->yoff;

	if(player->hook != NULL){
		p->val.i[10] = 1;
		p->val.f[11] = player->hook->ent->x;
		p->val.f[12] = player->hook->ent->y;
		p->val.f[13] = player->hook->ent->z;
	} else {
		p->val.i[10] = 0;
	}
	p->val.i[14] = player->blockMiningX;
	p->val.i[15] = player->blockMiningY;
	p->val.i[16] = player->blockMiningZ;
	p->val.i[17] = itm->ID;
	p->val.f[18] = player->hitOff;

	packetQueueToServer(p,15,19*4);
}

void decompressPacket(packet *p){
	uint8_t *t;
	uint8_t buf[1<<20];
	int len = LZ4_decompress_safe((const char *)&p->val.c, (char *)buf, packetLen(p), sizeof(buf));
	if(len <= 0){
		fprintf(stderr,"Decompression return %i\n",len);
		exit(1);
	}
	fprintf(stderr,"compLen: %i\nlen: %i\n",packetLen(p),len);
	fflush(stderr);
	for(t=buf;(t-buf)<len;t+=packetLen((packet *)t) + 4){
		clientParsePacket((packet *)t);
	}
}

void clientParsePacket(packet *p){
	const int pLen  = packetLen(p);
	const int pType = packetType(p);

	switch(pType){
		case 0: // Keepalive
		break;

		case 1: // requestPlayerSpawnPos
			characterSetPos(player,p->val.f[0],p->val.f[1],p->val.f[2]);
		break;

		case 2: // requestChungus
			fprintf(stderr,"Received a requestChungus packet from the server which should never happen.\n");
		break;

		case 3: // placeBlock
			worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],p->val.i[3]);
		break;

		case 4: // mineBlock
			if(worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],0)){
				fxBlockBreak(p->val.i[0],p->val.i[1],p->val.i[2],p->val.i[3]);
			}
		break;

		case 5: // Goodbye
			fprintf(stderr,"Received a Goodbye packet from the server which should never happen.\n");
		break;

		case 6: // blockMiningUpdate
			blockMiningUpdateFromServer(p);
		break;

		case 7:
			worldSetChungusLoaded(p->val.i[0],p->val.i[1],p->val.i[2]);
		break;

		case 8:
			characterGotHitBroadcast(p->val.i[1],p->val.f[0]);
		break;

		case 9:
			fprintf(stderr,"Received a PlayerJoin packet from the server which should never happen.\n");
		break;

		case 10:
			fprintf(stderr,"Received a itemDropNew packet from the server which should never happen.\n");
		break;

		case 11:
			fprintf(stderr,"Received a grenadeNew packet from the server which should never happen.\n");
		break;

		case 12:
			fprintf(stderr,"Received a beamblast packet from the server which should never happen.\n");
		break;

		case 13: // playerMoveDelta
			characterMoveDelta(player,p);
		break;

		case 14: // characterHit
			characterHitCheck(player,p->val.i[7],p->val.f[0],p->val.f[1],p->val.f[2],p->val.f[3],p->val.f[4],p->val.f[5],p->val.f[6]);
		break;

		case 15: // playerPos
			characterSetPlayerPos(p);
		break;

		case 16: // chatMsg
			chatParsePacket(p);
		break;

		case 17: // dyingMsg
			fprintf(stderr,"Received a dying message packet from the server which should never happen.\n");
		break;

		case 18:
			msgParseGetChunk(p);
		break;

		case 19: // setPlayerCount
			characterRemovePlayer(p->val.u[1],p->val.u[0]);
		break;

		case 20: // playerPickupItem
			characterPickupItem(player,p->val.s[0],p->val.s[1]);
		break;

		case 21: // itemDropUpdate
			itemDropUpdateFromServer(p);
		break;

		case 22: // grenadeExplode
			grenadeExplode(p->val.f[0],p->val.f[1],p->val.f[2],p->val.f[3],p->val.i[4]);
		break;

		case 23: // grenadeUpdate
			grenadeUpdateFromServer(p);
		break;

		case 24: // fxBeamBlaster
			fxBeamBlaster(p->val.f[0],p->val.f[1],p->val.f[2],p->val.f[3],p->val.f[4],p->val.f[5],p->val.f[6],p->val.i[7]);
		break;
		
		case 0xFF: // compressedMultiPacket
			decompressPacket(p);
		break;

		default:
			fprintf(stderr,"%i[%i] UNKNOWN PACKET\n",pType,pLen);
		break;
	}
}


void clientParse(){
	int off=0;
	if(recvBufLen == 0){return;}

	for(int max=128;max > 0;--max){
		if(off >= recvBufLen){break;}
		int pLen = packetLen((packet *)(recvBuf+off));
		if((pLen+4+off) > recvBufLen){
			break;
		}
		clientParsePacket((packet *)(recvBuf+off));
		fprintf(stderr,"pLen: %i[%i]\noff: %i\nrecvBufLen: %i\n",pLen,alignedLen(pLen),off,recvBufLen);
		fflush(stderr);
		off += alignedLen(pLen) + 4;
	}
	if(off < recvBufLen){
		for(int i=0;i<recvBufLen-off;i++){
			recvBuf[i] = recvBuf[i+off];
		}
	}
	recvBufLen -= off;
}

void clientSendIntroduction(){
	#ifndef __EMSCRIPTEN__
	queueToServer("NATIVE\r\n\r\n",10);
	#endif
}

void clientGreetServer(){
	clientSendIntroduction();
	msgPlayerJoinSendName(playerName);
	msgRequestPlayerSpawnPos();
}

void clientHandleEvents(){
	clientRead();
	clientParse();
	msgSendPlayerPos();
}

void clientGoodbye(){
	msgGoodbye();
	clientSendAllToServer();
}

void queueToServer(void *data, unsigned int len){
	if((sendBufLen + len) > sizeof(sendBuf)){
		clientSendAllToServer();
	}
	memcpy(sendBuf + sendBufLen, data, len);
	sendBufLen += len;
}
