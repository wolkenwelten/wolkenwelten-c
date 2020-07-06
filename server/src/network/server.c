#define _DEFAULT_SOURCE
#include "server.h"

#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../game/grenade.h"
#include "../main.h"
#include "../misc/sha1.h"
#include "../misc/misc.h"
#include "../misc/options.h"
#include "../network/messages.h"
#include "../network/packet.h"
#include "../voxel/bigchungus.h"

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
	packetLarge p;
	strncpy((char *)p.val.c,msg,sizeof(p.val.c)-1);
	p.val.c[sizeof(p.val.c)-1] = 0;
	packetQueueL(&p,2,-1);
	printf("%s[MSG]%s %s\n",termColors[6],termReset,msg);
}

void sendPlayerJoinMessage(int c){
	char msg[256];
	if((clientCount == 1) && optionSingleplayer){
		snprintf(msg,sizeof(msg),"Started Singleplayer Server [seed=%X]",optionWorldSeed);
		serverSendChatMsg(msg);
		return;
	}
	snprintf(msg,sizeof(msg),"%s joined",clients[c].playerName);
	serverSendChatMsg(msg);
}

void serverParseChatMsg(int c, packetLarge *m){
	char msg[256];
	snprintf(msg,sizeof(msg),"%s: %s",clients[c].playerName,(char *)m->val.c);
	serverSendChatMsg(msg);
}

void serverParsePlayerPos(int c, packetLarge *p){
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
	msgUpdatePlayer(c);
}

void serverParsePacketSmall(int c, packetSmall *p){
	int ptype = (p->ptype & (~0xC000));

	switch(ptype){
		case 0:
		break;

		case 1: // requestPlayerSpawnPos
			msgRequestPlayerSpawnPos(c,p);
			if(verbose){printf("[%02i] requestPlayerSpawnPos\n",c);}
		break;

		case 2: // requestChungus
			msgRequestChungus(c,p);
			if(verbose){printf("[%02i] requestChungus\n",c);}
		break;

		case 3: // placeBlock
			worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],p->target);
			sendToAllExcept(c,p,sizeof(packetSmall));
			if(verbose){printf("[%02i] placeBlock\n",c);}
		break;

		case 4: // mineBlock
			blockMiningDropItemsPos(p->val.i[0],p->val.i[1],p->val.i[2],worldGetB(p->val.i[0],p->val.i[1],p->val.i[2]));
			worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],0);
			sendToAllExcept(c,p,sizeof(packetSmall));
			if(verbose){printf("[%02i] mineBlock\n",c);}
		break;
		
		case 5: // Goodbye
			errno=0;
			serverKill(c);
		break;

		default:
			printf("%i S->%i\n",c,ptype);
		break;
	}
}

void serverParsePacketMedium(int c, packetMedium *p){
	int ptype = (p->ptype & (~0xC000));

	switch(ptype){
		case 0:
		break;

		case 1:
			memcpy(clients[c].playerName,p->val.c,sizeof(p->val.c));
			clients[c].playerName[sizeof(clients[c].playerName)-1] = 0;
			sendPlayerJoinMessage(c);
			if(verbose){printf("[%02i] sendPlayerName\n",c);}
		break;

		case 2:
			itemDropNewC(p);
			if(verbose){printf("[%02i] itemDropNewC\n",c);}
		break;

		case 3:
			grenadeNew(p);
			if(verbose){printf("[%02i] grenadeNew\n",c);}
		break;

		case 4:
			beamblast(c,p);
			if(verbose){printf("[%02i] beamblast\n",c);}
		break;

		default:
			printf("%i M->%i\n",c,ptype);
		break;
	}
}

void serverParsePacketLarge(int c, packetLarge *p){
	int ptype = (p->ptype & (~0xC000));

	switch(ptype){
		case 0:
		break;

		case 1:
			serverParsePlayerPos(c,p);
			//if(verbose){printf("[%02i] sendPlayerPos\n",c);}
		break;

		case 2:
			serverParseChatMsg(c,p);
			if(verbose){printf("[%02i] sendChatMsg\n",c);}
		break;

		default:
			printf("%i L->%i\n",c,ptype);
		break;
	}
}

void serverParsePacketHuge(int c, packetHuge *p){
	int ptype = (p->ptype & (~0xC000));
	printf("%i H->%i\n",c,ptype);

	switch(ptype){
		case 0:
		break;

		default:
			printf("%i H->%i\n",c,ptype);
		break;
	}
}

void serverParsePacket(int i){
	for(int max=16;max > 0;--max){
		if(clients[i].recvBufLen < 16){ return; }
		unsigned int pLen = packetLen(clients[i].recvBuf);
		if(pLen <= 0){ return; }
		if(pLen > clients[i].recvBufLen){ return; }

		switch(pLen){
			case sizeof(packetSmall):
				serverParsePacketSmall (i, (packetSmall *)  clients[i].recvBuf);
			break;
			case sizeof(packetMedium):
				serverParsePacketMedium(i, (packetMedium *) clients[i].recvBuf);
			break;
			case sizeof(packetLarge):
				serverParsePacketLarge (i, (packetLarge *)  clients[i].recvBuf);
			break;
			case sizeof(packetHuge):
				serverParsePacketHuge  (i, (packetHuge *)   clients[i].recvBuf);
			break;
		}

		if(clients[i].recvBufLen != pLen){
			for(unsigned int ii=0;ii < (clients[i].recvBufLen - pLen);++ii){
				clients[i].recvBuf[ii] = clients[i].recvBuf[ii+pLen];
			}
		}
		clients[i].recvBufLen -= pLen;
	}
}

void serverParseWSPacket(int i){
	for(int max=16;max > 0;--max){
		if(clients[i].recvWSBufLen < 16){ return; }
		uint8_t  opcode  = clients[i].recvWSBuf[0];
		uint8_t  masklen = clients[i].recvWSBuf[1];
		uint64_t mlen    = 0;
		uint8_t  mask[4];
		unsigned int ii=0;
		if(opcode != 0x82){
			fprintf(stderr,"oh noes: %X\n",opcode);
			serverKill(i);
		}
		if((masklen&0x80) == 0){
			//See: https://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-17
			fprintf(stderr,"Clients MUST mask: %X\n",masklen);
			serverKill(i);
		}
		if((masklen&0x7F) == 0x7E){
			mlen = (clients[i].recvWSBuf[2]<<8) | clients[i].recvWSBuf[3];
			ii = 4;
		}else if((masklen&0x7F) == 0x7F){
			// > 4GB Messages are not supported, therefore the upper 32-bits are ignored
			mlen |= clients[i].recvWSBuf[6] << 24;
			mlen |= clients[i].recvWSBuf[7] << 16;
			mlen |= clients[i].recvWSBuf[8] <<  8;
			mlen |= clients[i].recvWSBuf[9]      ;
			ii = 10;
		}else{
			mlen = masklen&0x7F;
			ii = 2;
		}
		mask[0] = clients[i].recvWSBuf[ii++];
		mask[1] = clients[i].recvWSBuf[ii++];
		mask[2] = clients[i].recvWSBuf[ii++];
		mask[3] = clients[i].recvWSBuf[ii++];

		for(uint64_t iii=0;iii<mlen;iii++){
			clients[i].recvBuf[clients[i].recvBufLen++] = clients[i].recvWSBuf[ii+iii] ^ mask[iii&3];
		}

		ii += mlen;
		if(clients[i].recvWSBufLen != ii){
			for(unsigned int iii=0;iii < (clients[i].recvWSBufLen - ii);++iii){
				clients[i].recvWSBuf[iii] = clients[i].recvWSBuf[iii+ii];
			}
		}
		clients[i].recvWSBufLen -= ii;
	}
}


void serverParseWebSocketHeaderField(int c,const char *key, const char *val){
	static SHA1_CTX ctx;
	static char buf[256];
	static uint8_t webSocketKeyHash[20];
	const char *b64hash;
	int len=0;
	if(strncmp(key,"Sec-WebSocket-Key",18) != 0){return;}
	len = snprintf(buf,sizeof(buf),"%s%s",val,"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	printf("Sec-WebSocket-Key = '%s'\nConcat: '%s'\n",val,buf);

	SHA1Init(&ctx);
	SHA1Update(&ctx,(unsigned char *)buf,len);
	SHA1Final(webSocketKeyHash,&ctx);
	b64hash = base64_encode(webSocketKeyHash,20,NULL);

	printf("SHA1 = ");
	for(int i=0;i<20;i++){
		printf("%X ",webSocketKeyHash[i]);
	}
	printf("\n");
	printf("B64 = %s\n",b64hash);

	len = snprintf(buf,sizeof(buf),"HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Protocol: binary\r\nSec-WebSocket-Accept: %s\r\n\r\n",b64hash);
	clients[c].flags &= !1;
	sendToClient(c,buf,len);
	serverSendClient(c);
	clients[c].flags |= 1;

}

void serverParseWebSocketHeader(int c,int end){
	int lb=0;
	char *key=NULL;
	char *val=NULL;
	for(int i=0;i<end;i++){
		if(clients[c].recvBuf[i] == '\n'){
			clients[c].recvBuf[i]=0;
			if(lb==5){
				serverParseWebSocketHeaderField(c,key,val);
				lb=0;
			}
			lb|=1;
			continue;
		}
		if(clients[c].recvBuf[i] == '\r'){
			clients[c].recvBuf[i]=0;
			if(lb==5){
				serverParseWebSocketHeaderField(c,key,val);
				lb=0;
			}
			lb|=2;
			continue;
		}
		if(lb==3){
			key = (char *) &clients[c].recvBuf[i];
			lb=4;
		}
		if(lb && (clients[c].recvBuf[i] == ':')){
			clients[c].recvBuf[i]=0;
			val = (char *) &clients[c].recvBuf[i+1];
			if(*val == ' '){val++;}
			lb=5;
		}
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
				if(clients[i].flags&1){
					serverParseWSPacket(i);
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

void dumpQueue(int c){
	static unsigned int lastSize=0;
	if(clients[c].sendBufLen <= lastSize){return;}
	lastSize = clients[c].sendBufLen;
	FILE *f = fopen("dump.dat","w");
	fwrite(clients[c].sendBuf,1,clients[c].sendBufLen,f);
	fclose(f);
}

void addQueuedChunks(int c){
	while(clients[c].sendBufLen < ((1<<22)-8192)){
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
	//dumpQueue(c);
}

void serverSend(){
	for(int i=0;i<clientCount;i++){
		if(clients[i].sendBufLen == 0){ continue; }
		serverSendClient(i);
	}
}

void serverHandleEvents(){
	serverAccept();
	serverRead();
	serverParse();
	serverKeepalive();
	serverSend();
}

int addWSMessagePrefix(uint8_t *d, int len, int maxlen){
	if(len < 126){
		if(maxlen <= 2){return 0;}
		*d++ = 0x82; // Opcode - Binary Data / Fin Bit
		*d++ = len;
		return 2;
	}else if(len < 0xFFFF){
		if(maxlen <= 4){return 0;}
		*d++ = 0x82; // Opcode - Binary Data / Fin Bit
		*d++ = 0x7E;
		*d++ = (len>>8) & 0xFF;
		*d++ = (len   ) & 0xFF;
		return 4;
	}else{
		if(maxlen < 10){return 0;}
		*d++ = 0x82; // Opcode - Binary Data / Fin Bit
		*d++ = 0x7F;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = 0;
		*d++ = (len>>24) & 0xFF;
		*d++ = (len>>16) & 0xFF;
		*d++ = (len>> 8) & 0xFF;
		*d++ = (len    ) & 0xFF;
		return 10;
	}
}

void sendToClient(int c,void *data,int len){
	int ret;
	int tlen = len;
	if(c < 0){return;}
	if(c >= clientCount){return;}
	if(clients[c].flags&1){
		tlen += 10;
	}

	while(clients[c].sendBufLen+tlen > (int)sizeof(clients[c].sendBuf)){
		ret = serverSendClient(c);
		if(ret == 1){
			usleep(10);
		}else if(ret == 2){
			return;
		}
	}
	if(clients[c].flags & 1){
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
