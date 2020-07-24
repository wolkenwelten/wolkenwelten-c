#define _DEFAULT_SOURCE
#include "chat.h"

#include "../gfx/gfx.h"
#include "../gui/textInput.h"
#include "../../../common/src/messages.h"
#include "../../../common/src/packet.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char chatLog[12][256];

void chatStartInput(){
	textInput(4, screenHeight-76, 256, 16, 1);
}

void msgSendChatMessage(char *msg){
	packet *p = (packet *)packetBuffer;
	p->val.s[0]=0;
	strncpy((char *)(p->val.c+2),msg,254);
	p->val.c[255] = 0;
	packetQueueToServer(p,16,256);
}

void chatCheckInput(){
	if(textInputActive)       {return;}
	if(textInputLock != 1)    {return;}

	textInputLock = 0;
	const char *msg = textInputGetBuffer();

	if(msg == NULL)           {return;}
	if(strnlen(msg,256) <= 0) {return;}

	msgSendChatMessage(textInputGetBuffer());
}

void chatParsePacket(packet *p){
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

void msgSendDyingMessage(char *msg, int c){
	packet *p = (packet *)packetBuffer;
	strncpy((char *)(p->val.c+2),msg,254);
	p->val.s[0] = c;
	p->val.c[255] = 0;
	packetQueueToServer(p,17,256);
}
