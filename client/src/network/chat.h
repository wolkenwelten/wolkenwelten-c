#pragma once
#include "../../../common/src/network/packet.h"

extern char chatLog[12][256];

void chatStartInput      ();
void chatCheckInput      ();
void chatParsePacket     (const packet *p);
void chatPrintDebug      (const char *msg);
void msgSendChatMessage  (const char *msg);
void msgSendDyingMessage (const char *msg, int c);
