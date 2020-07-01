#define _DEFAULT_SOURCE

#include "client.h"
#include "../main.h"
#include "../gui/menu.h"
#include "../gfx/effects.h"
#include "../game/blockMining.h"
#include "../game/entity.h"
#include "../game/grenade.h"
#include "../game/itemDrop.h"
#include "../misc/misc.h"
#include "../misc/options.h"
#include "../network/chat.h"
#include "../network/messages.h"
#include "../network/packet.h"
#include "../voxel/bigchungus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

char playerName[28];

int recvBufLen = 0;
uint8_t recvBuf[1<<22];

int sendBufSent = 0;
int sendBufLen = 0;
uint8_t sendBuf[1<<20];

size_t sentBytesCurrentSession = 0;
size_t recvBytesCurrentSession = 0;

int serverPort = 6309;
pid_t singlePlayerPID = 0;

#ifdef __MINGW32__
#include "client_win.h"
#else
#include "client_bsd.h"
#endif


void clientParsePacketSmall(packetSmall *p){
	const int ptype = (p->ptype & (~0xC000));
	switch(ptype){
		case 0: // Keepalive
		break;

		case 1: // requestPlayerSpawnPos
			characterSetPos(player,p->val.f[0],p->val.f[1],p->val.f[2]);
		break;

		case 2: // setClientCount
			characterSetPlayerCount(p->target);
		break;

		case 3: // placeBlock
			worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],p->target);
		break;

		case 4: // mineBlock
			if(worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],0)){
				fxBlockBreak(p->val.i[0],p->val.i[1],p->val.i[2],p->target);
			}
		break;

		case 5: // playerPickupItem
			characterPickupItem(player,p->val.i[0],p->val.i[1]);
		break;

		case 6: // blockMiningUpdate
			blockMiningUpdateFromServer(p);
		break;

		case 7:
			worldSetChungusLoaded(p->val.i[0],p->val.i[1],p->val.i[2]);
		break;

		default:
			printf("S->%i\n",ptype);
		break;
	}

}

void clientParsePacketMedium(packetMedium *p){
	int ptype = (p->ptype & (~0xC000));
	switch(ptype){
		case 0: // Keepalive
		break;

		case 1: // itemDropUpdate
			itemDropUpdateFromServer(p);
		break;

		case 2: // grenadeExplode
			grenadeExplode(p->val.f[0],p->val.f[1],p->val.f[2],p->val.f[3],p->target);
		break;

		case 3: // grenadeUpdate
			grenadeUpdateFromServer(p);
		break;

		case 4: // fxBeamBlaster
			fxBeamBlaster(p->val.f[0],p->val.f[1],p->val.f[2],p->val.f[3],p->val.f[4],p->val.f[5],p->val.f[6]);
		break;

		case 5: // playerMoveDelta
			characterMoveDelta(player,p);
		break;

		default:
			printf("M->%i\n",ptype);
		break;
	}
}

void clientParsePacketLarge(packetLarge *p){
	int ptype = (p->ptype & (~0xC000));
	switch(ptype){
		case 0: // Keepalive
		break;

		case 1: // playerPos
			characterSetPlayerPos(p->target,p);
		break;

		case 2: // chatMsg
			chatParsePacket(p);
		break;

		default:
			printf("L->%i\n",ptype);
		break;
	}
}

void clientParsePacketHuge(packetHuge *p){
	int ptype = (p->ptype & (~0xC000));
	switch(ptype){
		case 0:
		break;

		case 3:
			msgParseGetChunk(p);
		break;

		default:
			printf("S->%i\n",ptype);
		break;
	}
}

void clientParsePacket(void *p, int pLen){
	switch(pLen){
		case sizeof(packetSmall):
			clientParsePacketSmall((packetSmall *)p);
		break;
		case sizeof(packetMedium):
			clientParsePacketMedium((packetMedium *)p);
		break;
		case sizeof(packetLarge):
			clientParsePacketLarge((packetLarge *)p);
		break;
		case sizeof(packetHuge):
			clientParsePacketHuge((packetHuge *)p);
		break;
	}
}


void clientParse(){
	int off=0;
	if(recvBufLen == 0){return;}

	while(off < recvBufLen){
		int pLen = packetLen(recvBuf+off);
		if(pLen <= 0){ return; }
		if((pLen+off) > recvBufLen){
			for(int i=0;i<recvBufLen-off;i++){
				recvBuf[i] = recvBuf[i+off];
			}
			break;
		}
		clientParsePacket(recvBuf+off,pLen);
		off += pLen;
	}
	recvBufLen -= off;
}

void clientSendName(){
	packetMedium p;
	clientGetName();
	strncpy((char *)p.val.c,playerName,28);
	packetQueueM(&p,1);
}

void clientSendIntroduction(){
	#ifndef __EMSCRIPTEN__
	queueToServer("NATIVE\r\n\r\n",10);
	#endif
}

void clientGreetServer(){
	clientSendIntroduction();
	clientSendName();
	msgRequestPlayerSpawnPos();
}

void clientHandleEvents(){
	clientRead();
	clientParse();
	msgSendPlayerPos();
}

void queueToServer(void *data, unsigned int len){
	if((sendBufLen + len) > sizeof(sendBuf)){
		clientSendAllToServer();
	}
	memcpy(sendBuf + sendBufLen, data, len);
	sendBufLen += len;
}
