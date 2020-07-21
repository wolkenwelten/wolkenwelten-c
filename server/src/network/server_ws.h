#pragma once
#include <stdint.h>

const char *base64_encode            (const unsigned char *data,unsigned int input_length,const char *prefix);
void serverParseWebSocketPacket      (int c);
void serverParseWebSocketHeaderField (int c,const char *key, const char *val);
void serverParseWebSocketHeader      (int c,int end);
void serverParseWebSocket            (int c,int end);
int  addWSMessagePrefix              (uint8_t *d, int len, int maxlen);
