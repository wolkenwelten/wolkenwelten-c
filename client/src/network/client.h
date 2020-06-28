#pragma once
#include <unistd.h>
#include <sys/types.h>

void clientInit();
void clientFree();
void clientGreetServer();
void clientSendAllToServer();
void closeSingleplayerServer();

void clientHandleEvents();
void queueToServer(void *data, unsigned int len);
