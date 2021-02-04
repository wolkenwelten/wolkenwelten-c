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
	packetQueueToServer(p,msgtChatMsg,256);

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
	packetQueueToServer(p,msgtDyingMsg,256);
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
