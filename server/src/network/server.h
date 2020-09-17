#pragma once
#include "../game/character.h"

typedef struct {
	char playerName[32];
	character *c;
	int state;
	int flags;

	unsigned int animalUpdateOffset;
	unsigned int itemDropUpdateOffset;
	unsigned int itemDropPriorityQueueLen;
	uint16_t itemDropPriorityQueue[128];

	unsigned int chngReqQueueLen;
	uint64_t chngReqQueue[128];
	unsigned int chnkReqQueueLen;
	uint64_t chnkReqQueue[4096];

	long long unsigned int socket;

	unsigned int recvBufLen;
	uint8_t recvBuf[1<<16];

	unsigned int recvWSBufLen;
	uint8_t recvWSBuf[1<<16];

	unsigned int sendBufSent;
	unsigned int sendBufLen;
	uint8_t sendBuf[1<<20];
} clientConnection;

#define STATE_READY      0
#define STATE_CONNECTING 1
#define STATE_CLOSED     2
#define STATE_INTRO      3

#define CONNECTION_WEBSOCKET (1   )
#define CONNECTION_DO_UPDATE (1<<1)

extern clientConnection clients[32];
extern int clientCount;

const char *getPlayerLeaveMessage(int c);
void serverInit            ();
void serverFree            ();
void serverHandleEvents    ();
void serverInitClient      (int c);
void sendToAll             (const void *data, int len);
void sendToAllExcept       (int e, const void *data, int len);
void sendToClient          (int c, const void *data, int len);
void addChungusToQueue     (int c, uint16_t x, uint16_t y, uint16_t z);
void addQueuedChunks       (int c);
void addPriorityItemDrop   (uint16_t i);
void serverSendChatMsg     (const char *msg);

int  getClientByName       (const char *name);
int  serverSendClient      (int c);
void serverAccept          ();
void serverRead            ();
void serverCloseClient     (int c);
void serverKill            (int c);
void serverParsePacket     (int c);
