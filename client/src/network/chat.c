/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "chat.h"
#include "../misc/options.h"
#include "../../../common/src/common.h"

#include "../gfx/gfx.h"
#include "../gui/textInput.h"
#include "../../../common/src/network/messages.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *chatLog[12];
char *chatHistory[8];
int  chatHistoryCount =  0;
int  chatHistorySel   = -1;

void chatEmpty(){
	for(int i=0;i < 12;i++){
		free(chatLog[i]);
		chatLog[i] = NULL;
	}
}

void chatSendRaw(const char *msg){
	size_t len = strnlen(msg,4096);
	memcpy(packetBuffer.v.u8,msg,len+1);
	packetQueueToServer(&packetBuffer, msgtChatMsg, alignedLen(len+1));
}

void chatSend(const char *msg){
	char tmp[4096];
	const size_t len = snprintf(tmp,sizeof(tmp),"%s%s%s: %s",ansiFG[10],playerName,ansiRS,msg);
	if(len == 0){
		fprintf(stderr,"Couldn't send message %s",msg);
		return;
	}
	chatSendRaw(tmp);
	char *buf = malloc(len+1);
	snprintf(buf,len,"%s",packetBuffer.v.u8);

	free(chatHistory[7]);
	for(int i=7;i>0;i--){
		chatHistory[i] = chatHistory[i-1];
	}
	if(++chatHistoryCount > 8){chatHistoryCount = 8;}
	chatHistory[0] = buf;
}

static void chatAddMessage(const char *msg){
	size_t len = strnlen(msg,4096);
	char *buf = malloc(len+1);
	snprintf(buf,len+1,"%s",msg);

	for(int i=0;i<11;i++){
		chatLog[i] = chatLog[i+1];
	}
	chatLog[11] = buf;
}

void chatParsePacket(const packet *p){
	chatAddMessage((void *)p->v.u8);
}

void chatPrintDebug(const char *msg){
	chatAddMessage(msg);
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
