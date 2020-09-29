#define _GNU_SOURCE

#include "chat.h"
#include "../../../common/src/common.h"

#include "../gfx/gfx.h"
#include "../gui/textInput.h"
#include "../../../common/src/network/messages.h"

#include <string.h>
#include <stdio.h>

char chatLog[12][256];

void chatEmpty(){
	for(int i=0;i < 12;i++){
		*chatLog[i]=0;
	}
}

void msgSendChatMessage(const char *msg){
	packet *p = &packetBuffer;
	p->val.s[0]=0;
	strncpy((char *)(p->val.c+2),msg,254);
	p->val.c[255] = 0;
	packetQueueToServer(p,16,256);
}

void chatParsePacket(const packet *p){
	for(int i=0;i<11;i++){
		memcpy(chatLog[i],chatLog[i+1],256);
	}
	strncpy(chatLog[11],(char *)(p->val.c+2),254);
	chatLog[11][255]=0;
}

void chatPrintDebug(const char *msg){
	for(int i=0;i<11;i++){
		memcpy(chatLog[i],chatLog[i+1],256-1);
	}
	strncpy(chatLog[11],msg,256-1);
	chatLog[11][255]=0;
}

void msgSendDyingMessage(const char *msg, int c){
	packet *p = &packetBuffer;
	strncpy((char *)(p->val.c+2),msg,254);
	p->val.s[0] = c;
	p->val.c[255] = 0;
	packetQueueToServer(p,17,256);
}
