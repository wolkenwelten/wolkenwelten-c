#pragma once
#include "../game/character.h"

typedef struct {
	char playerName[32];
	character *c;
	int state;
	int flags;
	
	unsigned int itemDropUpdateOffset;

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

#define CONNECTION_WEBSOCKET (1<<0)

extern clientConnection clients[32];
extern int clientCount;

char *getPlayerLeaveMessage(int c);
void serverInit            ();
void serverFree            ();
void serverHandleEvents    ();
void sendToAll             (void *data, int len);
void sendToAllExcept       (int e, void *data, int len);
void sendToClient          (int c, void *data, int len);
void addChungusToQueue     (int c, uint16_t x, uint16_t y, uint16_t z);
void addQueuedChunks       (int c);
void serverSendChatMsg     (const char *msg);

int  serverSendClient      (int c);
void serverAccept          ();
void serverRead            ();
void serverCloseClient     (int c);
void serverKill            (int c);
void serverParsePacket     (int c);
