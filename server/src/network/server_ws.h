#pragma once
#include "../../../common/src/common.h"

const char *base64_encode            (const u8 *data,uint input_length,const char *prefix);
void serverParseWebSocketPacket      (uint c);
void serverParseWebSocketHeaderField (uint c,const char *key, const char *val);
void serverParseWebSocketHeader      (uint c,uint end);
int  addWSMessagePrefix              (u8 *d, uint len, uint maxlen);
