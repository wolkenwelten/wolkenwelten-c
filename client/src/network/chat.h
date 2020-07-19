#pragma once
#include "../../../common/src/packet.h"

extern char chatLog[12][256];

void chatStartInput();
void chatCheckInput();
void chatParsePacket(packet *p);
void chatPrintDebug(const char *msg);
void msgSendChatMessage(char *msg);
void msgSendDyingMessage(char *msg, int c);
