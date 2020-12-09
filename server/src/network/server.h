#pragma once
#include "../../../common/src/common.h"

typedef struct {
	u8 x,y,z,w;
} chungusReqEntry;

typedef struct {
	u16 x,y,z,w;
} chunkReqEntry;

typedef struct {
	char playerName[32];
	character *c;
	int state,flags;
	u64 socket,lastPing;

	uint fireUpdateOffset;
	uint waterUpdateOffset;
	uint projectileUpdateOffset;

	uint animalUpdateOffset;
	uint animalPriorityUpdateOffset;
	uint animalUpdateWindowSize;

	uint itemDropPriorityQueueLen;
	u16  itemDropPriorityQueue[128];
	uint itemDropUpdateOffset;
	uint itemDropUpdateWindowSize;

	uint chngReqQueueLen;
	chungusReqEntry chngReqQueue[128];
	uint chnkReqQueueLen,chnkUpdateIter;
	chunkReqEntry chnkReqQueue[8192];


	uint recvBufOff;
	uint recvBufLen;
	u8   recvBuf[1<<16];

	uint recvWSBufLen;
	u8   recvWSBuf[1<<16];

	uint sendBufSent;
	uint sendBufLen;
	uint sendBufLastCompressed;
	u8   sendBuf[1<<22];
} clientConnection;

#define STATE_READY      0
#define STATE_CONNECTING 1
#define STATE_CLOSED     2
#define STATE_INTRO      3

#define CONNECTION_WEBSOCKET (1   )
#define CONNECTION_DO_UPDATE (1<<1)

extern clientConnection clients[32];
extern uint clientCount;

const char *getPlayerLeaveMessage(uint c);
void serverInit            ();
void serverFree            ();
void serverHandleEvents    ();
void serverInitClient      (uint c);
void sendToAll             (const void *data, uint len);
void sendToAllExcept       (uint e, const void *data, uint len);
void sendToClient          (uint c, const void *data, uint len);
void addChungusToQueue     (uint c, u8 x, u8 y, u8 z);
void addQueuedChunks       (uint c);
void addPriorityItemDrop   (u16  i);
void delPriorityItemDrop   (u16  i);
void serverSendChatMsg     (const char *msg);
void msgSendChunk          (uint c, const chunk *chnk);

int  getClientByName       (const char *name);
int  getClientByCharacter  (const character *c);
uint getClientLatency      (uint c);
uint serverSendRaw         (uint c, void *p, uint len);
int  serverSendClient      (uint c);
void serverAccept          ();
void serverRead            ();
void serverCloseClient     (uint c);
void serverKill            (uint c);
void serverParsePacket     (uint c);
