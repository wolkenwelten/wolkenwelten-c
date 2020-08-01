#define _DEFAULT_SOURCE
#include "command.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../network/server.h"
#include "../../../common/src/network/messages.h"

char replyBuf[256];

static void cmdDie(int c, const char *cmd){
	int cmdLen = strnlen(cmd,252);
	if((cmdLen > 3) && (cmd[3] == ' ')){
		int target = getClientByName(cmd+4);
		if(target >= 0){
			msgPlayerDamage(target,1000);
		}else{
			snprintf(replyBuf,sizeof(replyBuf),".die : Can't find '%s'\n",cmd+4);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
		}
		return;
	}
	msgPlayerDamage(c,1000);
}

int parseCommand(int c, const char *cmd){
	if(cmd[0] != '.'){return 0;}
	const char *tcmp = cmd+1;
	
	if(strncmp(tcmp,"die",3) == 0){
		cmdDie(c,tcmp);
		return 1;
	}
		
	fprintf(stderr,"CMD [%s]\n",cmd);
	return 1;
}