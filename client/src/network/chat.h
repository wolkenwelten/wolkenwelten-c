#pragma once
#include "../../../common/src/network/packet.h"

extern char *chatLog[12];

void        chatEmpty           ();
void        chatParsePacket     (const packet *p);
void        chatPrintDebug      (const char *msg);
void        chatSend            (const char *msg);
void        chatSendRaw         (const char *msg);
const char *chatGetPrevHistory  ();
const char *chatGetNextHistory  ();
void        chatResetHistorySel ();
