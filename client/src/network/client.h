#pragma once
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>

extern char playerName[28]; 
extern size_t sentBytesCurrentSession;
extern size_t recvBytesCurrentSession;

void clientInit();
void clientFree();
void clientGetName();
void clientGoodbye();
void clientGreetServer();
void clientSendAllToServer();
void closeSingleplayerServer();

void clientHandleEvents();
void queueToServer(void *data, unsigned int len);
