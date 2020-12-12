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
#include "../game/rope.h"
#include "../game/projectile.h"
#include "../game/time.h"
#include "../game/water.h"
#include "../misc/lisp.h"
#include "../misc/options.h"
#include "../sdl/sdl.h"
#include "../network/chat.h"
#include "../../../common/src/misc/lz4.h"
#include "../../../common/src/misc/misc.h"
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

void msgParseGetChunk(const packet *p){
	u16 x = p->v.u16[2048];
	u16 y = p->v.u16[2049];
	u16 z = p->v.u16[2050];
	chungus *chng =  worldGetChungus(x>>8,y>>8,z>>8);
	if(chng == NULL){return;}
	chunk *chnk = chungusGetChunkOrNew(chng,x,y,z);
	if(chnk == NULL){return;}
	memcpy(chnk->data,p->v.u8,sizeof(chnk->data));
	chnk->ready &= ~15;
}

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
	packetQueueToServer(p,15,pLen);
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

	switch(pType){
		case 0: // Keepalive
			break;
		case 1: // playerPos
			characterSetPos(player,vecNewP(&p->v.f[0]));
			characterSetRot(player,vecNewP(&p->v.f[3]));
			characterSetVelocity(player,vecZero());
			characterFreeHook(player);
			player->flags &= ~CHAR_SPAWNING;
			break;
		case 2: // requestChungus
			fprintf(stderr,"Received a requestChungus packet from the server which should never happen.\n");
			break;
		case 3: // placeBlock
			fprintf(stderr,"Received a placeBlock packet from the server which should never happen.\n");
			break;
		case 4: // mineBlock
			fxBlockBreak(vecNew(p->v.u16[0],p->v.u16[1],p->v.u16[2]),p->v.u16[3]);
			break;
		case 5: // Goodbye
			fprintf(stderr,"Received a Goodbye packet from the server which should never happen.\n");
			break;
		case 6: // blockMiningUpdate
			blockMiningUpdateFromServer(p);
			break;
		case 7:
			worldSetChungusLoaded(p->v.u8[0],p->v.u8[1],p->v.u8[2]);
			break;
		case 8:
			// ToDo: beingGotHit
			dispatchBeingGotHit(p);
			break;
		case 9: // setTime
			gtimeSetTime(p->v.u32[0]);
			break;
		case 10:
			fprintf(stderr,"Received an itemDropNew msg from the server, this should never happen.\n");
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
		case 14: // characterName
			characterSetName(p);
			break;
		case 15: // playerPos
			characterUpdatePacket(p);
			break;
		case 16: // chatMsg
			chatParsePacket(p);
			break;
		case 17: // dyingMsg
			fprintf(stderr,"Received a dying message packet from the server which should never happen.\n");
			break;
		case 18: // chunkData
			msgParseGetChunk(p);
			break;
		case 19: // setPlayerCount
			characterRemovePlayer(p->v.u16[1],p->v.u16[0]);
			break;
		case 20: // playerPickupItem
			characterPickupPacket(player,p);
			break;
		case 21: // itemDropDel
			fprintf(stderr,"Received an itemDropDel msg from the server, this should never happen.\n");
			break;
		case 22: // explode
			explode(vecNewP(&p->v.f[0]),((float)p->v.u16[6])/256.f,p->v.u16[7]);
			break;
		case 23: // grenadeUpdate
			grenadeUpdateFromServer(p);
			break;
		case 24: // fxBeamBlaster
			fxBeamBlaster(vecNewP(&p->v.f[0]),vecNewP(&p->v.f[3]),p->v.f[6],p->v.f[7]);
			break;
		case 25: // msgItemDropUpdate
			itemDropUpdateFromServer(p);
			break;
		case 26: // msgPlayerDamage
			characterDamagePacket(player,p);
			break;
		case 28:
			characterSetData(player,p);
			break;
		case 29:
			characterSetInventoryP(player,p);
			break;
		case 30:
			animalSyncFromServer(p);
			break;
		case 31:
			fprintf(stderr,"Received an dirtyChunk msg from the server, this should never happen.\n");
			break;
		case 32:
			fprintf(stderr,"Received an animalDmg msg from the server, this should never happen.\n");
			break;
		case 33:
			handlePingPong();
			msgPingPong(-1);
			break;
		case 34:
			fxAnimalDiedPacket(p);
			break;
		case 35:
			characterSetEquipmentP(player,p);
			break;
		case 36:
			fprintf(stderr,"Received an itemDropPickup msg from the server, this should never happen.\n");
			break;
		case 37:
			ropeUpdateP(p);
			break;
		case 38:
			projectileRecvUpdate(-1,p);
			break;
		case 39:
			fxProjectileHit(p);
			break;
		case 40:
			fireRecvUpdate(-1,p);
			break;
		case 41:
			waterRecvUpdate(-1,p);
			break;
		case 42:
			lispRecvSExpr(p);
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
