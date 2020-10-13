#define _GNU_SOURCE

#include "chat.h"
#include "../../../common/src/common.h"

#include "../gfx/gfx.h"
#include "../gui/textInput.h"
#include "../../../common/src/network/messages.h"

#include <string.h>
#include <stdio.h>

char chatLog[12][256];

char chatHistory[8][256];
int  chatHistoryCount =  0;
int  chatHistorySel   = -1;

void chatEmpty(){
	for(int i=0;i < 12;i++){
		*chatLog[i]=0;
	}
}

void msgSendChatMessage(const char *msg){
	packet *p = &packetBuffer;
	p->v.u16[0]=0;
	strncpy((char *)(&p->v.u8[2]),msg,254);
	p->v.u8[255] = 0;
	packetQueueToServer(p,16,256);

	for(int i=7;i>0;i--){
		memcpy(chatHistory[i],chatHistory[i-1],256);
	}
	strncpy(chatHistory[0],msg,256);
	chatHistory[0][255]=0;
	if(++chatHistoryCount > 8){chatHistoryCount = 8;}
}

void chatParsePacket(const packet *p){
	for(int i=0;i<11;i++){
		memcpy(chatLog[i],chatLog[i+1],256);
	}
	memcpy(chatLog[11],(char *)(&p->v.u8[2]),254);
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
	strncpy((char *)(&p->v.u8[2]),msg,254);
	p->v.u16[0]  = c;
	p->v.u8[255] = 0;
	packetQueueToServer(p,17,256);
}

const char *chatGetPrevHistory(){
	if(++chatHistorySel > MIN(7,chatHistoryCount-1)){chatHistorySel = 0;}
	return chatHistory[chatHistorySel];
}

const char *chatGetNextHistory(){
	if(--chatHistorySel < 0){chatHistorySel = MIN(7,chatHistoryCount-1);}
	return chatHistory[chatHistorySel];
}

void chatResetHistorySel(){
	chatHistorySel = -1;
}
