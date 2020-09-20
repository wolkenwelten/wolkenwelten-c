#pragma once
#include "../game/character.h"

typedef struct {
	char playerName[32];
	character *c;
	int state;
	int flags;

	uint animalUpdateOffset;
	uint itemDropUpdateOffset;
	uint itemDropPriorityQueueLen;
	u16  itemDropPriorityQueue[128];

	uint chngReqQueueLen;
	u64  chngReqQueue[128];
	uint chnkReqQueueLen;
	u64  chnkReqQueue[4096];

	long long unsigned int socket;

	uint recvBufLen;
	u8   recvBuf[1<<16];

	uint recvWSBufLen;
	u8   recvWSBuf[1<<16];

	uint sendBufSent;
	uint sendBufLen;
	u8   sendBuf[1<<20];
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
void addChungusToQueue     (uint c, u16 x, u16 y, u16 z);
void addQueuedChunks       (uint c);
void addPriorityItemDrop   (u16  i);
void delPriorityItemDrop   (u16  i);
void serverSendChatMsg     (const char *msg);

int  getClientByName       (const char *name);
int  serverSendClient      (uint c);
void serverAccept          ();
void serverRead            ();
void serverCloseClient     (uint c);
void serverKill            (uint c);
void serverParsePacket     (uint c);
