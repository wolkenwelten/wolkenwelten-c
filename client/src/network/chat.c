#define _DEFAULT_SOURCE
#include "chat.h"
#include "../gfx/gfx.h"
#include "../gui/textInput.h"
#include "../network/packet.h"
#include <string.h>

char chatLog[12][256];

void chatStartInput(){
	textInput(4, screenHeight-76, 256, 16, 1);
}

void msgSendChatMessage(char *msg){
	packetLarge p;
	strncpy((char *)p.val.c,msg,sizeof(p.val.c)-1);
	p.val.c[sizeof(p.val.c)-1] = 0;
	packetQueueL(&p,2);
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

void chatParsePacket(packetLarge *p){
	for(int i=0;i<11;i++){
		memcpy(chatLog[i],chatLog[i+1],256);
	}
	strncpy(chatLog[11],(char *)p->val.c,256);
}

void chatPrintDebug(const char *msg){
	for(int i=0;i<11;i++){
		memcpy(chatLog[i],chatLog[i+1],256-1);
	}
	strncpy(chatLog[11],msg,256-1);
}
