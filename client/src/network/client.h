#pragma once
#include "../../../common/src/network/packet.h"

#include <stddef.h>

extern bool   mayTryToStartServer;
extern bool   goodbyeSent;
extern uint   lastPing;
extern uint   lastLatency;
extern uint   connectionState;
extern size_t sentBytesCurrentSession;
extern size_t recvBytesCurrentSession;
extern size_t recvUncompressedBytesCurrentSession;

void  clientInit               ();
void  clientFree               ();
void  clientGetName            ();
void  clientGoodbye            ();
void  clientGreetServer        ();
void  clientTranceive          ();
void  clientParse              ();
void  closeSingleplayerServer  ();
void  msgSendPlayerPos         ();
void  decompressPacket         (const packet *p);
void  clientParsePacket        (const packet *p);
void  queueToServer            (const void *data, uint len);
char *clientGetServerExecutable();
