#include "command.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/entity.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/time.h"
#include "../game/water.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include "../../../common/nujel/nujel.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char replyBuf[256];
lClosure *clRoot;


lVal *wwlnfUpdateAll(lClosure *c, lVal *v){
	(void)c;(void)v;
	worldSetAllUpdated();
	return lValNil();
}
lVal *wwlnfACount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(animalUsedCount);
}
lVal *wwlnfFCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(fireCount);
}
lVal *wwlnfBMCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(blockMiningGetActive());
}
lVal *wwlnfIDCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(itemDropGetActive());
}
lVal *wwlnfECount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(entityCount);
}
lVal *wwlnfWCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(waterCount);
}
lVal *wwlnfPX(lClosure *c, lVal *v){
	lVal *sym = lValSym("pid");
	if(v == NULL){v = lResolveSym(c,sym->vSymbol);}
	v = lEval(c,v);
	if(v == NULL)             {return lValNil();}
	if(v->type != ltInt)      {return lValNil();}
	const int cc = v->vInt;
	if((cc < 0) || (cc >= 32)){return lValNil();}
	if(clients[cc].state)     {return lValNil();}
	if(clients[cc].c == NULL) {return lValNil();}
	return lValInt((int)clients[cc].c->pos.x);
}
lVal *wwlnfPY(lClosure *c, lVal *v){
	lVal *sym = lValSym("pid");
	if(v == NULL){v = lResolveSym(c,sym->vSymbol);}
	v = lEval(c,v);
	if(v == NULL)             {return lValNil();}
	if(v->type != ltInt)      {return lValNil();}
	const int cc = v->vInt;
	if((cc < 0) || (cc >= 32)){return lValNil();}
	if(clients[cc].state)     {return lValNil();}
	if(clients[cc].c == NULL) {return lValNil();}
	return lValInt((int)clients[cc].c->pos.y);
}
lVal *wwlnfPZ(lClosure *c, lVal *v){
	lVal *sym = lValSym("pid");
	if(v == NULL){v = lResolveSym(c,sym->vSymbol);}
	v = lEval(c,v);
	if(v == NULL)             {return lValNil();}
	if(v->type != ltInt)      {return lValNil();}
	const int cc = v->vInt;
	if((cc < 0) || (cc >= 32)){return lValNil();}
	if(clients[cc].state)     {return lValNil();}
	if(clients[cc].c == NULL) {return lValNil();}
	return lValInt((int)clients[cc].c->pos.z);
}
lVal *wwlnfSetB(lClosure *c, lVal *v){
	int args[4] = {-1,-1,-1,0};
	for(int i=0;i<4;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){return lValNil();}
		args[i] = t->vInt;
	}
	worldSetB(args[0],args[1],args[2],(u8)args[3]);
	return lValInt(args[3]);
}

lVal *wwlnfGetB(lClosure *c, lVal *v){
	int args[3] = {-1,-1,-1};
	for(int i=0;i<3;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){return lValNil();}
		args[i] = t->vInt;
	}
	return lValInt(worldGetB(args[0],args[1],args[2]));
}

lVal *wwlnfBox(lClosure *c, lVal *v){
	int args[7] = {-1,-1,-1,-1,-1,-1,0};
	for(int i=0;i<7;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){return lValNil();}
		args[i] = t->vInt;
	}
	worldBox(args[0],args[1],args[2],args[3],args[4],args[5],(u8)args[6]);
	return lValInt(args[6]);
}

lVal *wwlnfMBox(lClosure *c, lVal *v){
	int args[6] = {-1,-1,-1,-1,-1,-1};
	for(int i=0;i<6;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){return lValNil();}
		args[i] = t->vInt;
	}
	worldBoxMine(args[0],args[1],args[2],args[3],args[4],args[5]);
	return lValBool(true);
}

lVal *wwlnfSphere(lClosure *c, lVal *v){
	int args[5] = {-1,-1,-1,-1,0};
	for(int i=0;i<5;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){return lValNil();}
		args[i] = t->vInt;
	}
	worldBoxSphere(args[0],args[1],args[2],args[3],(u8)args[4]);
	return lValInt(args[4]);
}
lVal *wwlnfMSphere(lClosure *c, lVal *v){
	int args[4] = {-1,-1,-1,-1};
	for(int i=0;i<4;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){return lValNil();}
		args[i] = t->vInt;
	}
	worldBoxMineSphere(args[0],args[1],args[2],args[3]);
	return lValBool(true);
}

lVal *wwlnfGive(lClosure *c, lVal *v){
	int args[3] = {1,1,-1};
	lVal *sym = lValSym("pid");
	lVal *pid = lResolveSym(c,sym->vSymbol);
	pid = lEval(c,pid);
	args[2] = pid->vInt;
	for(int i=0;i<3;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){break;}
		args[i] = t->vInt;
	}
	msgPickupItem(args[2],(item){args[0],args[1]});
	return lValInt(args[1]);
}

lVal *wwlnfDmg(lClosure *c, lVal *v){
	int args[2] = {4,-1};
	lVal *sym = lValSym("pid");
	lVal *pid = lResolveSym(c,sym->vSymbol);
	pid = lEval(c,pid);
	args[1] = pid->vInt;
	for(int i=0;i<2;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){break;}
		args[i] = t->vInt;
	}
	msgBeingDamage(args[1],args[0],0,beingCharacter(args[1]),-1,vecZero());
	return lValInt(args[0]);
}

lVal *wwlnfDie(lClosure *c, lVal *v){
	int args[1] = {-1};
	lVal *sym = lValSym("pid");
	lVal *pid = lResolveSym(c,sym->vSymbol);
	pid = lEval(c,pid);
	args[0] = pid->vInt;
	for(int i=0;i<1;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){break;}
		args[i] = t->vInt;
	}
	msgBeingDamage(args[0],1000,0,beingCharacter(args[0]),-1,vecZero());
	return lValInt(args[0]);
}

lVal *wwlnfNewAnim(lClosure *c, lVal *v){
	int args[5] = {1,1,-1,-1,-1};
	for(int i=0;i<5;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){return lValNil();}
		args[i] = t->vInt;
	}
	vec cpos = vecNew(args[2],args[3],args[4]);
	for(int i=0;i<args[1];i++){
		animalNew(cpos,args[0],-1);
	}
	return lValInt(args[1]);
}

lVal *wwlnfSetAnim(lClosure *c, lVal *v){
	int args[6] = {-1,-1,-1,-1,-2,-1};
	for(int i=0;i<6;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){continue;}
		args[i] = t->vInt;
	}
	const int ai = args[0];
	if((ai < 0) || (ai > (int)animalCount)){return lValBool(false);}
	animal *a = &animalList[ai];
	if(a->type ==  0){return lValBool(false);}
	if(args[1] >=  0){a->hunger    = args[1];}
	if(args[2] >=  0){a->sleepy    = args[2];}
	if(args[3] >= -1){a->pregnancy = args[3];}
	if(args[4] >=  0){a->state     = args[4];}
	if(args[5] >=  0){a->health    = args[5];}
	return lValBool(true);
}

lVal *wwlnfTp(lClosure *c, lVal *v){
	int args[4] = {-1,-1,-1,-1};
	lVal *sym = lValSym("pid");
	lVal *pid = lResolveSym(c,sym->vSymbol);
	pid = lEval(c,pid);
	args[3] = pid->vInt;
	for(int i=0;i<4;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){break;}
		args[i] = t->vInt;
	}
	vec cpos = vecNew(args[0],args[1],args[2]);
	if(!vecInWorld(cpos)){return lValBool(false);}
	msgPlayerSetPos(args[3], cpos, vecZero());
	return lValBool(true);
}

lVal *wwlnfWater(lClosure *c, lVal *v){
	int args[4] = {-1,-1,-1,-1};
	args[3] = 8192;
	for(int i=0;i<4;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){break;}
		args[i] = t->vInt;
	}
	waterNewF(args[0],args[1],args[2],args[3]);
	return lValInt(args[3]);
}

void initCommands(){
	lInit();
	clRoot = lClosureNew(NULL);
	lVal *sym = lValSym("pid");
	lVal *pid = lDefineClosureSym(clRoot, sym->vSymbol);
	pid->type = ltInt;
	pid->vInt = 123;
	lClosureAddNF(clRoot,"update", &wwlnfUpdateAll);
	lClosureAddNF(clRoot,"acount", &wwlnfACount);
	lClosureAddNF(clRoot,"fcount", &wwlnfFCount);
	lClosureAddNF(clRoot,"bmcount",&wwlnfBMCount);
	lClosureAddNF(clRoot,"idcount",&wwlnfIDCount);
	lClosureAddNF(clRoot,"ecount", &wwlnfECount);
	lClosureAddNF(clRoot,"px",     &wwlnfPX);
	lClosureAddNF(clRoot,"py",     &wwlnfPY);
	lClosureAddNF(clRoot,"pz",     &wwlnfPZ);
	lClosureAddNF(clRoot,"setb",   &wwlnfSetB);
	lClosureAddNF(clRoot,"getb",   &wwlnfGetB);
	lClosureAddNF(clRoot,"box",    &wwlnfBox);
	lClosureAddNF(clRoot,"sphere", &wwlnfSphere);
	lClosureAddNF(clRoot,"mbox",   &wwlnfMBox);
	lClosureAddNF(clRoot,"msphere",&wwlnfMSphere);
	lClosureAddNF(clRoot,"give",   &wwlnfGive);
	lClosureAddNF(clRoot,"dmg",    &wwlnfDmg);
	lClosureAddNF(clRoot,"die",    &wwlnfDie);
	lClosureAddNF(clRoot,"newAnim",&wwlnfNewAnim);
	lClosureAddNF(clRoot,"setAnim",&wwlnfSetAnim);
	lClosureAddNF(clRoot,"tp",     &wwlnfTp);
	lClosureAddNF(clRoot,"water",  &wwlnfWater);
	lClosureAddNF(clRoot,"wcount", &wwlnfWCount);
}

void freeCommands(){
	lClosureFree(clRoot);
}

static void cmdLisp(int c,const char *str, u8 id){
	static char reply[512];
	memset(reply,0,sizeof(reply));
	lVal *sym = lValSym("pid");
	lVal *pid = lResolveClosureSym(clRoot, sym->vSymbol);
	pid->vInt = c;
	lVal *v = NULL;
	for(lVal *sexpr = lParseSExprCS(str); sexpr != NULL; sexpr = sexpr->next){
		v = lEval(clRoot,sexpr);
	}
	lSPrintChain(v,reply,&reply[sizeof(reply)]);
	for(uint i=0;i<sizeof(reply);i++){if(reply[i] == '\n'){reply[i] = ' ';}}
	lClosureGC();

	if(id == 0){
		serverSendChatMsg(reply);
	}else{
		msgLispSExpr(c, id, reply);
	}
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
	newInventory[i++] = itemNew(282, 1);
	newInventory[i++] = itemNew(283, 1);
	newInventory[i++] = itemNew(270, 1);
	newInventory[i++] = itemNew(271, 1);
	newInventory[i++] = itemNew(258,42);
	newInventory[i++] = itemNew(256,99);

	newInventory[i++] = itemNew(265,999);
	newInventory[i++] = itemNew(265,999);
	newInventory[i++] = itemNew(265,999);

	newInventory[i++] = itemNew(284,999);
	newInventory[i++] = itemNew(284,999);
	newInventory[i++] = itemNew(284,999);

	i=0;
	newEquipment[i++] = itemNew(274,1);
	newEquipment[i++] = itemNew(275,1);
	newEquipment[i++] = itemNew(276,1);

	msgPlayerSetInventory(target,newInventory,40);
	msgPlayerSetEquipment(target,newEquipment, 3);
}

static void cmdTime(int c, const char *cmd){
	(void)c;
	gtimeSetTimeOfDayHRS(cmd + 5);
	msgSetTime(-1, gtimeGetTime());
	snprintf(replyBuf,sizeof(replyBuf),"It is now %s",gtimeGetTimeOfDayHRS(gtimeGetTimeOfDay()));
	replyBuf[sizeof(replyBuf)-1]=0;
	serverSendChatMsg(replyBuf);
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

int parseCommand(int c, const char *cmd){
	if(cmd[0] != '.'){return 0;}
	const char *tcmp = cmd+1;

	if(strncmp(tcmp,"help",4) == 0){
		cmdHelp(c,tcmp);
		return 1;
	}
	if(strncmp(tcmp,"dbgitem",7) == 0){
		cmdDbgitem(c,tcmp);
		return 1;
	}
	if(strncmp(tcmp,"time",4) == 0){
		cmdTime(c,tcmp);
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

	cmdLisp(c,tcmp,0);
	return 1;
}

void lispRecvSExpr(int c,const packet *p){
	u8 id = p->v.u8[0];
	const char *str = (const char *)&p->v.u8[1];
	cmdLisp(c,str,id);
}
