#include "command.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../network/server.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char replyBuf[256];

static void cmdDmg(int c, const char *cmd){
	int dmg = 4;
	int target = c;
	int argc;
	char **argv;

	argv = splitArgs(cmd,&argc);
	if(argc > 1){
		int tmp = getClientByName(argv[1]);
		if(tmp >= 0){
			target = tmp;
		}else{
			snprintf(replyBuf,sizeof(replyBuf),".dmg : Can't find '%s'",cmd+4);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
		}
	}
	if(argc > 2){
		dmg = atoi(argv[2]);
	}

	msgBeingDamage(target,dmg,0,beingCharacter(target),-1,vecZero());
}

static void cmdDie(int c, const char *cmd){
	int cmdLen = strnlen(cmd,252);
	int target = c;
	if((cmdLen > 3) && (cmd[3] == ' ')){
		target = getClientByName(cmd+4);
		if(target < 0){
			snprintf(replyBuf,sizeof(replyBuf),".die : Can't find '%s'",cmd+4);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
		}
	}
	msgBeingDamage(target,1000,0,beingCharacter(target),-1,vecZero());
}

static void cmdDbgitem(int c, const char *cmd){
	int cmdLen = strnlen(cmd,252);
	int target = c;
	item newInventory[40];
	item newEquipment[3];
	memset(newInventory,0,sizeof(newInventory));
	memset(newEquipment,0,sizeof(newEquipment));
	if((cmdLen > 3) && (cmd[3] == ' ')){
		target = getClientByName(cmd+4);
		if(target < 0){
			snprintf(replyBuf,sizeof(replyBuf),".dbgitem : Can't find '%s'",cmd+4);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
		}
	}
	int i=0;
	newInventory[i++] = itemNew(261, 1);
	newInventory[i++] = itemNew(262, 1);
	newInventory[i++] = itemNew(263, 1);
	newInventory[i++] = itemNew(264, 1);
	newInventory[i++] = itemNew(270, 1);
	newInventory[i++] = itemNew(271, 1);
	newInventory[i++] = itemNew(258,42);
	newInventory[i++] = itemNew(256,99);
	newInventory[i++] = itemNew(265,999);

	i=0;
	newEquipment[i++] = itemNew(274,1);
	newEquipment[i++] = itemNew(275,1);
	newEquipment[i++] = itemNew(276,1);

	msgPlayerSetInventory(target,newInventory,40);
	msgPlayerSetEquipment(target,newEquipment, 3);
}

static void cmdHeal(int c, const char *cmd){
	int dmg = 128;
	int target = c;
	int argc;
	char **argv;

	argv = splitArgs(cmd,&argc);
	if(argc > 1){
		int tmp = getClientByName(argv[1]);
		if(tmp >= 0){
			target = tmp;
		}else{
			snprintf(replyBuf,sizeof(replyBuf),".heal : Can't find '%s'",argv[1]);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
		}
	}
	if(argc > 2){
		dmg = atoi(argv[2]);
	}

	msgBeingDamage(target,-dmg,0,beingCharacter(target),-1,vecZero());
}

static void cmdGive(int c, const char *cmd){
	int amount = 1;
	int id     = 0;
	int target = c;
	int argc;
	char **argv;

	argv = splitArgs(cmd,&argc);
	if(argc == 1){
		snprintf(replyBuf,sizeof(replyBuf),".give ID [PLAYER] [AMOUNT]");
		replyBuf[sizeof(replyBuf)-1]=0;
		serverSendChatMsg(replyBuf);
		return;
	}
	if(argc > 1){
		id = atoi(argv[1]);
		if(id <= 0){
			snprintf(replyBuf,sizeof(replyBuf),".give: error with ID %i",id);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
			return;
		}
	}
	if(argc > 2){
		int tmp = getClientByName(argv[2]);
		if(tmp >= 0){
			target = tmp;
		}
	}
	if(argc > 3){
		amount = atoi(argv[3]);
		if(amount <= 0){
			snprintf(replyBuf,sizeof(replyBuf),".give: error with amount %i",amount);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
			return;
		}
	}

	msgPickupItem(target,(item){id,amount});
}

static void cmdTp(int c, const char *cmd){
	(void)c;
	int argc;
	char **argv;
	int target = c;

	argv = splitArgs(cmd,&argc);
	if(argc != 4){
		snprintf(replyBuf,sizeof(replyBuf),".tp X Y Z [PLAYER] : You need to pass 3 integer values");
		replyBuf[sizeof(replyBuf)-1]=0;
		serverSendChatMsg(replyBuf);
		return;
	}
	if(argc > 4){
		int tmp = getClientByName(argv[4]);
		if(tmp >= 0){
			target = tmp;
		}
	}
	const vec pos = vecNew(atof(argv[1]),atof(argv[2]),atof(argv[3]));
	msgPlayerSetPos(target,pos,vecZero());
}

static void cmdTpr(int c, const char *cmd){
	(void)c;
	int argc;
	char **argv;
	int target = c;

	argv = splitArgs(cmd,&argc);
	if(argc < 4){
		snprintf(replyBuf,sizeof(replyBuf),".tpr X Y Z [PLAYER] : You need to pass 3 integer values");
		replyBuf[sizeof(replyBuf)-1]=0;
		serverSendChatMsg(replyBuf);
		return;
	}
	if(argc > 4){
		int tmp = getClientByName(argv[4]);
		if(tmp >= 0){
			target = tmp;
		}
	}
	const vec pos = vecNew(atof(argv[1]),atof(argv[2]),atof(argv[3]));
	msgPlayerSetPos(target,vecAdd(pos,clients[target].c->pos),vecZero());
}

void cmdAni(int c, const char *cmd){
	int amount = 1;
	int id     = 1;
	int target = c;
	int argc;
	char **argv;

	argv = splitArgs(cmd,&argc);
	if(argc > 1){
		id = atoi(argv[1]);
		if(id <= 0){
			snprintf(replyBuf,sizeof(replyBuf),".ani: error with ID %i",id);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
			return;
		}
	}
	if(argc > 2){
		amount = atoi(argv[2]);
		if(amount <= 0){
			snprintf(replyBuf,sizeof(replyBuf),".ani: error with amount %i",amount);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
			return;
		}
	}
	if(argc > 3){
		int tmp = getClientByName(argv[3]);
		if(tmp >= 0){
			target = tmp;
		}
	}

	character *ch = clients[target].c;
	if(ch == NULL){return;}
	for(;amount > 0;amount--){
		animalNew(vecAdd(ch->pos,vecDegToVec(ch->rot)),id,-1);
	}
}

void cmdHelp(int c, const char *cmd){
	(void)c;
	(void)cmd;
	serverSendChatMsg(" --=-- HELP --=--");
	serverSendChatMsg(".help = This help message");
	serverSendChatMsg(".ani  = Spawns an animal");
	serverSendChatMsg(".dmg [PLAYER] [DMG] = Damages a player");
	serverSendChatMsg(".dbgitem [PLAYER] = Gives a player some things");
	serverSendChatMsg(".die [PLAYER] = Kills a player");
	serverSendChatMsg(".heal [PLAYER] [HEAL] = Heals a player");
	serverSendChatMsg(".give ID [PLAYER] [QTY] = Heals a player");
	serverSendChatMsg(".tpr X Y Z [PLAYER} = Teleports a player to a relative position");
	serverSendChatMsg(".tp X Y Z [PLAYER} = Teleports a player to an absolute position");
}

static const char *colorSignalHigh(int err, int warn, int good, int v){
	if(v <= err) {return ansiFG[ 9];}
	if(v <= warn){return ansiFG[11];}
	if(v >= good){return ansiFG[10];}
	return ansiFG[15];
}
/*
static const char *colorSignalLow(int err, int warn, int good, int v){
	if(v >= err) {return ansiFG[ 9];}
	if(v >= warn){return ansiFG[11];}
	if(v <= good){return ansiFG[10];}
	return ansiFG[15];
}*/

int parseCommand(int c, const char *cmd){
	if(cmd[0] != '.'){return 0;}
	const char *tcmp = cmd+1;

	if(strncmp(tcmp,"help",4) == 0){
		cmdHelp(c,tcmp);
		return 1;
	}
	if(strncmp(tcmp,"ani",3) == 0){
		cmdAni(c,tcmp);
		return 1;
	}

	if(strncmp(tcmp,"dmg",3) == 0){
		cmdDmg(c,tcmp);
		return 1;
	}

	if(strncmp(tcmp,"dbgitem",3) == 0){
		cmdDbgitem(c,tcmp);
		return 1;
	}

	if(strncmp(tcmp,"die",3) == 0){
		cmdDie(c,tcmp);
		return 1;
	}

	if(strncmp(tcmp,"heal",4) == 0){
		cmdHeal(c,tcmp);
		return 1;
	}

	if(strncmp(tcmp,"give",4) == 0){
		cmdGive(c,tcmp);
		return 1;
	}

	if(strncmp(tcmp,"tpr",3) == 0){
		cmdTpr(c,tcmp);
		return 1;
	}

	if(strncmp(tcmp,"tp",2) == 0){
		cmdTp(c,tcmp);
		return 1;
	}

	if(strncmp(tcmp,"acount",6) == 0){
		snprintf(replyBuf,sizeof(replyBuf),".acount : %i",animalCount);
		replyBuf[sizeof(replyBuf)-1]=0;
		serverSendChatMsg(replyBuf);
		return 1;
	}

	if(strncmp(tcmp,"bmcount",7) == 0){
		snprintf(replyBuf,sizeof(replyBuf),".bmcount : %i",blockMiningGetActive());
		replyBuf[sizeof(replyBuf)-1]=0;
		serverSendChatMsg(replyBuf);
		return 1;
	}

	if(strncmp(tcmp,"idcount",7) == 0){
		snprintf(replyBuf,sizeof(replyBuf),".idcount : %i",itemDropGetActive());
		replyBuf[sizeof(replyBuf)-1]=0;
		serverSendChatMsg(replyBuf);
		return 1;
	}

	if(strncmp(tcmp,"ecount",6) == 0){
		snprintf(replyBuf,sizeof(replyBuf),".ecount : %i",entityCount);
		replyBuf[sizeof(replyBuf)-1]=0;
		serverSendChatMsg(replyBuf);
		return 1;
	}

	if(strncmp(tcmp,"deadanimals",11) == 0){
		for(uint i = animalCount/2;i<animalCount;i++){
			animalRDie(&animalList[i]);
			animalDel(i);
		}
		snprintf(replyBuf,sizeof(replyBuf),".deadanimals");
		replyBuf[sizeof(replyBuf)-1]=0;
		serverSendChatMsg(replyBuf);
		return 1;
	}

	if(strncmp(tcmp,"wsize",5) == 0){
		const uint aws  = clients[c].animalUpdateWindowSize;
		const uint idws = clients[c].itemDropUpdateWindowSize;
		snprintf(replyBuf,sizeof(replyBuf),"[A:%s%i%s][ID:%s%i%s]",
			colorSignalHigh(2,4, 8, aws),aws, ansiFG[15],
			colorSignalHigh(4,8,16,idws),idws,ansiFG[15]
			);
		replyBuf[sizeof(replyBuf)-1]=0;
		serverSendChatMsg(replyBuf);
		return 1;
	}

	fprintf(stderr,"CMD [%s]\n",cmd);
	return 1;
}
