#include "command.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/entity.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/time.h"
#include "../game/water.h"
#include "../game/weather.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include "../../../common/src/nujel/nujel.h"
#include "../../../common/src/nujel/arithmetic.h"
#include "../../../common/src/nujel/string.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char replyBuf[256];
lClosure *clRoot;


lVal *wwlnfMsPerTick(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		if(t->vInt > 0){
			msPerTick = t->vInt;
		}
	}
	return lValInt(msPerTick);
}
lVal *wwlnfUpdateAll(lClosure *c, lVal *v){
	(void)c;(void)v;
	worldSetAllUpdated();
	return lValNil();
}
lVal *wwlnfACount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(animalCount);
}
lVal *wwlnfAUCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(animalUsedCount);
}
lVal *wwlnfARCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	uint rcount = 0;
	for(uint i=0;i<countof(animalList);i++){
		if(animalList[i].type){rcount++;}
	}
	return lValInt(rcount);
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
	msgBeingDamage(args[1],args[0],0,1.f,beingCharacter(args[1]),-1,vecZero());
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
	msgBeingDamage(args[0],1000,0,1.f,beingCharacter(args[0]),-1,vecZero());
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

lVal *wwlnfWSrc(lClosure *c, lVal *v){
	int args[3] = {-1,-1,-1};
	for(int i=0;i<3;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v);
		v = v->next;
		if(t->type != ltInt){break;}
		args[i] = t->vInt;
	}
	waterSource = vecNew(args[0],args[1],args[2]);
	return lValInt(args[0]);
}

lVal *wwlnfNoAggro(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if(t != NULL){
		bool na = true;
		switch(t->type){
		default: break;
		case ltInt:  na = t->vInt != 0; break;
		case ltBool: na = t->vBool;     break;
		case ltNil:  na = false;        break;
		}
		animalNoAggro = na;
	}
	return lValBool(animalNoAggro);
}

lVal *wwlnfCDen(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lEval(c,v);
		if(t != NULL){
			t = lnfInt(c,t);
			cloudsSetDensity(t->vInt);
		}
	}
	return lValInt(cloudDensityMin);
}

lVal *wwlnfRain(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lEval(c,v);
		if(t != NULL){
			t = lnfInt(c,t);
			weatherSetRainDuration(t->vInt);
		}
	}
	return lValInt(rainDuration);
}

lVal *wwlnfLShed(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	chungusFreeOldChungi(100);

	return lValBool(true);
}

lVal *wwlnfProf(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	return lValString(profGetReport());
}

lVal *wwlnfChungi(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValInt(chungusCount - chungusFreeCount);
}

lVal *wwlnfWVel(lClosure *c, lVal *v){
	float args[3] = {windGVel.x,windGVel.y,windGVel.z};
	for(int i=0;i<3;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v);
		t = lnfFloat(c,t);
		args[i] = t->vFloat;
		v = v->next;
	}
	cloudsSetWind(vecNew(args[0],args[1],args[2]));
	return lValBool(true);
}

lVal *wwlnfDelW(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	for(uint i=waterCount-1;i<waterCount;i--){
		waterDel(i);
	}
	return lValBool(true);
}

void lispEvalNR(const char *str){
	for(lVal *sexpr = lParseSExprCS(str); sexpr != NULL; sexpr = sexpr->next){
		lEval(clRoot,sexpr);
	}
}

void initCommands(){
	lInit();
	clRoot = lClosureNew(NULL);
	lVal *sym = lValSym("pid");
	lVal *pid = lDefineClosureSym(clRoot, sym->vSymbol);
	pid->type = ltInt;
	pid->vInt = 123;
	lClosureAddNF(clRoot,"mst",    &wwlnfMsPerTick);
	lClosureAddNF(clRoot,"noaggro",&wwlnfNoAggro);
	lClosureAddNF(clRoot,"update", &wwlnfUpdateAll);
	lClosureAddNF(clRoot,"acount", &wwlnfACount);
	lClosureAddNF(clRoot,"aucount",&wwlnfAUCount);
	lClosureAddNF(clRoot,"arcount",&wwlnfARCount);
	lClosureAddNF(clRoot,"fcount", &wwlnfFCount);
	lClosureAddNF(clRoot,"bmcount",&wwlnfBMCount);
	lClosureAddNF(clRoot,"idcount",&wwlnfIDCount);
	lClosureAddNF(clRoot,"ecount", &wwlnfECount);
	lClosureAddNF(clRoot,"chungi", &wwlnfChungi);
	lClosureAddNF(clRoot,"lshed",  &wwlnfLShed);
	lClosureAddNF(clRoot,"prof",   &wwlnfProf);
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
	lClosureAddNF(clRoot,"wsrc",   &wwlnfWSrc);
	lClosureAddNF(clRoot,"cden",   &wwlnfCDen);
	lClosureAddNF(clRoot,"wvel",   &wwlnfWVel);
	lClosureAddNF(clRoot,"delw",   &wwlnfDelW);
	lClosureAddNF(clRoot,"rain",   &wwlnfRain);
	lispEvalNR("(define abs (lambda (a) (cond ((< a 0) (- 0 a)) (#t a))))");
	lispEvalNR("(define heal (lambda (a) (- (dmg (cond (a (- a)) (#t -20))))))");
	lispEvalNR("(define wtest (lambda () (water (px) (py) (pz)) (water (+ (px) 1) (py) (pz)) (water (+ (px) 1) (py) (+ (pz) 1)) (water (px) (py) (+ (pz) 1)) ))");
	lispEvalNR("(define wu (lambda () (wsrc (px) (+ (py) 8) (pz))))");
}

void freeCommands(){
	lClosureFree(clRoot);
}

static void cmdLisp(int c,const char *str, u8 id){
	static char reply[8192];
	memset(reply,0,sizeof(reply));
	lVal *sym = lValSym("pid");
	lVal *pid = lResolveClosureSym(clRoot, sym->vSymbol);
	pid->vInt = c;
	lVal *v = NULL;
	for(lVal *sexpr = lParseSExprCS(str); sexpr != NULL; sexpr = sexpr->next){
		v = lEval(clRoot,sexpr);
	}
	lSPrintChain(v,reply,&reply[sizeof(reply)-1]);
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
	newInventory[i++] = itemNew(283, 1);
	newInventory[i++] = itemNew(288, 1);
	newInventory[i++] = itemNew(282, 1);
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

int parseCommand(int c, const char *cmd){
	if(cmd[0] != '.'){return 0;}
	const char *tcmp = cmd+1;

	if(strncmp(tcmp,"dbgitem",7) == 0){
		cmdDbgitem(c,tcmp);
		return 1;
	}
	if(strncmp(tcmp,"time",4) == 0){
		cmdTime(c,tcmp);
		return 1;
	}
	if(strncmp(tcmp,"deadanimals",11) == 0){
		//for(uint i = animalCount/2;i<animalCount;i++){
		for(uint i = animalCount-1;i < animalCount;i--){
			if(animalList[i].type == 0){continue;}
			//animalRDie(&animalList[i]);
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
