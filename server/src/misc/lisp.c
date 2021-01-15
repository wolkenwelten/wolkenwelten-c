#include "lisp.h"

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
#include "../../../common/src/misc/lisp.h"
#include "../../../common/src/network/messages.h"

#include "../../../common/src/nujel/nujel.h"
#include "../../../common/src/nujel/casting.h"
#include "../../../common/src/nujel/reader.h"
#include "../../../common/src/nujel/string.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char replyBuf[256];
lClosure *clRoot;

static uint getPID(lClosure *c){
	lVal *sym = lValSym("pid");
	lVal *pid = lResolveClosureSym(c, sym->vSymbol);
	if(pid == NULL){return 123;}
	return pid->vInt;
}

static void setPID(lClosure *c, uint pid){
	lVal *sym = lValSym("pid");
	lVal *t = lDefineClosureSym(c, sym->vSymbol);
	t->vList.car = lValInt(pid);
}

void lPrintError(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
}

static lVal *wwlnfUpdateAll(lClosure *c, lVal *v){
	(void)c;(void)v;
	worldSetAllUpdated();
	return lValNil();
}
static lVal *wwlnfACount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(animalCount);
}
static lVal *wwlnfAUCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(animalUsedCount);
}
static lVal *wwlnfARCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	uint rcount = 0;
	for(uint i=0;i<countof(animalList);i++){
		if(animalList[i].type){rcount++;}
	}
	return lValInt(rcount);
}
static lVal *wwlnfFCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(fireCount);
}
static lVal *wwlnfBMCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(blockMiningGetActive());
}
static lVal *wwlnfIDCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(itemDropGetActive());
}
static lVal *wwlnfECount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(entityCount);
}
static lVal *wwlnfWCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(waterCount);
}

static lVal *wwlnfPlayerPos(lClosure *c, lVal *v){
	(void)v;
	const int cc = getPID(c);
	if((cc < 0) || (cc >= 32)){return lValNil();}
	if(clients[cc].state)     {return lValNil();}
	if(clients[cc].c == NULL) {return lValNil();}
	return lValVec(clients[cc].c->pos);
}

static lVal *wwlnfSetB(lClosure *c, lVal *v){
	vec pos = vecNOne();
	u8 b = 0;

	for(int i=0;i<2;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		switch(i){
		case 0:
			t = lnfVec(c,t);
			pos = t->vVec;
			break;
		case 1:
			t = lnfInt(c,t);
			b = t->vInt;
			break;
		}
	}

	worldSetB(pos.x,pos.y,pos.z,b);
	return lValInt(b);
}

static lVal *wwlnfGetB(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValInt(0);}
	t = lnfVec(c,v);
	if(t == NULL){return lValInt(0);}
	return lValInt(worldGetB(t->vVec.x,t->vVec.y,t->vVec.z));
}

static lVal *wwlnfBox(lClosure *c, lVal *v){
	vec arg[2] = {vecZero(),vecOne()};
	u8 b = 0;

	for(int i=0;i<3;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		switch(i){
		case 0:
		case 1:
			t = lnfVec(c,t);
			arg[i] = t->vVec;
			break;
		case 2:
			t = lnfInt(c,t);
			b = t->vInt;
			break;
		}
	}
	worldBox(arg[0].x,arg[0].y,arg[0].z,arg[0].x,arg[0].y,arg[0].z,(u8)b);
	return lValInt(b);
}

static lVal *wwlnfMBox(lClosure *c, lVal *v){
	vec arg[2] = {vecNOne(),vecOne()};
	for(int i=0;i<2;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		t = lnfVec(c,v);
		arg[i] = t->vVec;
		break;
	}
	worldBoxMine(arg[0].x,arg[0].y,arg[0].z,arg[1].x,arg[1].y,arg[1].z);
	return lValBool(true);
}

static lVal *wwlnfSphere(lClosure *c, lVal *v){
	vec pos = vecNOne();
	float r = 1.f;
	u8 b = 0;

	for(int i=0;i<3;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		switch(i){
		case 0:
			t = lnfVec(c,t);
			pos = t->vVec;
			break;
		case 1:
			t = lnfFloat(c,t);
			b = t->vFloat;
			break;
		case 2:
			t = lnfInt(c,t);
			b = t->vInt;
			break;
		}
	}
	worldBoxSphere(pos.x,pos.y,pos.z,r,(u8)b);
	return lValInt(b);
}

static lVal *wwlnfMSphere(lClosure *c, lVal *v){
	vec pos = vecNOne();
	float r = 4.f;

	for(int i=0;i<2;i++){
		if(v == NULL){return lValNil();}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		switch(i){
		case 0:
			t = lnfVec(c,t);
			pos = t->vVec;
			break;
		case 1:
			t = lnfFloat(c,t);
			r = t->vFloat;
			break;
		}
	}

	worldBoxMineSphere(pos.x,pos.y,pos.z,r);
	return lValBool(true);
}

static lVal *wwlnfGive(lClosure *c, lVal *v){
	int args[3] = {1,1,-1};
	args[2] = getPID(c);
	for(int i=0;i<3;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		if(t->type != ltInt){break;}
		args[i] = t->vInt;
	}
	msgPickupItem(args[2],(item){args[0],args[1]});
	return lValInt(args[1]);
}

static lVal *wwlnfDmg(lClosure *c, lVal *v){
	int args[2] = {4,-1};
	args[1] = getPID(c);
	for(int i=0;i<2;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		if(t->type != ltInt){break;}
		args[i] = t->vInt;
	}
	msgBeingDamage(args[1],args[0],0,1.f,beingCharacter(args[1]),-1,vecZero());
	return lValInt(args[0]);
}

static lVal *wwlnfDie(lClosure *c, lVal *v){
	int args[1] = {-1};
	args[0] = getPID(c);
	for(int i=0;i<1;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		if(t->type != ltInt){break;}
		args[i] = t->vInt;
	}
	msgBeingDamage(args[0],1000,0,1.f,beingCharacter(args[0]),-1,vecZero());
	return lValInt(args[0]);
}

static lVal *wwlnfNewAnim(lClosure *c, lVal *v){
	vec pos;
	uint amount = 1;
	uint type = 1;

	for(int i=0;i<3;i++){
		if(v == NULL){
			if(i == 0){
				return lValNil();
			}else{
				break;
			}
		}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		switch(i){
		case 0:
			t = lnfVec(c,t);
			pos = t->vVec;
			break;
		case 1:
			t = lnfInt(c,t);
			type = t->vInt;
			break;
		case 2:
			t = lnfInt(c,t);
			amount = t->vInt;
			break;
		}
	}
	if(type <= 0){return lValNil();}
	for(uint i=0;i<amount;i++){
		animalNew(pos,type,-1);
	}
	return lValInt(amount);
}

static lVal *wwlnfSetAnim(lClosure *c, lVal *v){
	int args[6] = {-1,-1,-1,-1,-2,-1};
	for(int i=0;i<6;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
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

static lVal *wwlnfTp(lClosure *c, lVal *v){
	vec pos = vecNOne();
	uint playerid = getPID(c);
	for(int i=0;i<2;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		switch(i){
		case 0:
			t = lnfVec(c,t);
			pos = t->vVec;
			break;
		case 1:
			t = lnfInt(c,t);
			playerid = t->vInt;
			break;
		}
	}
	if(!vecInWorld(pos)){return lValBool(false);}
	character *tpc = clients[playerid].c;
	if(tpc != NULL){
		msgPlayerSetPos(playerid, pos, tpc->rot);
	}else{
		msgPlayerSetPos(playerid, pos, vecZero());
	}

	return lValBool(true);
}

static lVal *wwlnfWaterNew(lClosure *c, lVal *v){
	vec pos = vecNOne();
	int amount = 8192;

	for(int i=0;i<2;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		switch(i){
		case 0:
			t = lnfVec(c,t);
			pos = t->vVec;
			break;
		case 1:
			t = lnfInt(c,t);
			amount = t->vInt;
			break;
		}
	}
	waterNewF(pos.x,pos.y,pos.z,amount);
	return lValInt(amount);
}

static lVal *wwlnfNoAggro(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
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

static lVal *wwlnfCDen(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lEval(c,v->vList.car);
		if(t != NULL){
			t = lnfInt(c,t);
			cloudsSetDensity(t->vInt);
		}
	}
	return lValInt(cloudGDensityMin);
}

static lVal *wwlnfRain(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lEval(c,v->vList.car);
		if(t != NULL){
			t = lnfInt(c,t);
			weatherSetRainDuration(t->vInt);
		}
	}
	return lValInt(rainIntensity);
}

static lVal *wwlnfTime(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lEval(c,v->vList.car);
		if(t != NULL){
			if(t->type == ltString){
				gtimeSetTimeOfDayHRS(t->vString->buf);
			}else{
				t = lnfInt(c,t);
				gtimeSetTime(t->vInt);
			}
			msgSetTime(-1, gtimeGetTime());
		}
	}
	return lValString(gtimeGetTimeOfDayHRS(gtimeGetTimeOfDay()));
}

static lVal *wwlnfLShed(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	chungusFreeOldChungi(100);
	for(uint i = animalCount/2-1;i < animalCount;i--){
		if(animalList[i].type == 0){continue;}
		animalDel(i);
	}

	return lValBool(true);
}

static lVal *wwlnfChungi(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValInt(chungusCount - chungusFreeCount);
}

static lVal *wwlnfWVel(lClosure *c, lVal *v){
	vec nvel = vecZero();
	if(v != NULL){
		lVal *t = lnfVec(c,lEval(c,v));
		if(t != NULL){
			nvel = t->vVec;
		}
	}
	cloudsSetWind(nvel);
	return lValVec(windVel);
}

static lVal *wwlnfDbgItem(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	int target = getPID(c);
	item newInventory[40];
	item newEquipment[3];
	memset(newInventory,0,sizeof(newInventory));
	memset(newEquipment,0,sizeof(newEquipment));

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

	return lValBool(true);
}

static void lispEvalNR(const char *str){
	lEval(clRoot,lWrap(lRead(str)));
}

lVal *lResolveNativeSym(const lSymbol s){
	if(strcmp(s.c,"player-pos") == 0)        {return lValNativeFunc(wwlnfPlayerPos);}
	if(strcmp(s.c,"no-aggro") == 0)          {return lValNativeFunc(wwlnfNoAggro);}
	if(strcmp(s.c,"update-all") == 0)        {return lValNativeFunc(wwlnfUpdateAll);}
	if(strcmp(s.c,"animal-count") == 0)      {return lValNativeFunc(wwlnfACount);}
	if(strcmp(s.c,"animal-used") == 0)       {return lValNativeFunc(wwlnfAUCount);}
	if(strcmp(s.c,"animal-real") == 0)       {return lValNativeFunc(wwlnfARCount);}
	if(strcmp(s.c,"fire-count") == 0)        {return lValNativeFunc(wwlnfFCount);}
	if(strcmp(s.c,"water-count") == 0)       {return lValNativeFunc(wwlnfWCount);}
	if(strcmp(s.c,"mining-count") == 0)      {return lValNativeFunc(wwlnfBMCount);}
	if(strcmp(s.c,"item-drop-count") == 0)   {return lValNativeFunc(wwlnfIDCount);}
	if(strcmp(s.c,"entity-count") == 0)      {return lValNativeFunc(wwlnfECount);}
	if(strcmp(s.c,"chungus-count") == 0)     {return lValNativeFunc(wwlnfChungi);}
	if(strcmp(s.c,"load-shed") == 0)         {return lValNativeFunc(wwlnfLShed);}
	if(strcmp(s.c,"give") == 0)              {return lValNativeFunc(wwlnfGive);}
	if(strcmp(s.c,"dmg") == 0)               {return lValNativeFunc(wwlnfDmg);}
	if(strcmp(s.c,"die") == 0)               {return lValNativeFunc(wwlnfDie);}
	if(strcmp(s.c,"animal-new") == 0)        {return lValNativeFunc(wwlnfNewAnim);}
	if(strcmp(s.c,"animal-set") == 0)        {return lValNativeFunc(wwlnfSetAnim);}
	if(strcmp(s.c,"water-new") == 0)         {return lValNativeFunc(wwlnfWaterNew);}
	if(strcmp(s.c,"cloud-threshold") == 0)   {return lValNativeFunc(wwlnfCDen);}
	if(strcmp(s.c,"wind-velocity") == 0)     {return lValNativeFunc(wwlnfWVel);}
	if(strcmp(s.c,"rain-set") == 0)          {return lValNativeFunc(wwlnfRain);}
	if(strcmp(s.c,"setb") == 0)              {return lValNativeFunc(wwlnfSetB);}
	if(strcmp(s.c,"getb") == 0)              {return lValNativeFunc(wwlnfGetB);}
	if(strcmp(s.c,"box") == 0)               {return lValNativeFunc(wwlnfBox);}
	if(strcmp(s.c,"sphere") == 0)            {return lValNativeFunc(wwlnfSphere);}
	if(strcmp(s.c,"mbox") == 0)              {return lValNativeFunc(wwlnfMBox);}
	if(strcmp(s.c,"msphere") == 0)           {return lValNativeFunc(wwlnfMSphere);}
	if(strcmp(s.c,"time") == 0)              {return lValNativeFunc(wwlnfTime);}
	if(strcmp(s.c,"tp") == 0)                {return lValNativeFunc(wwlnfTp);}
	if(strcmp(s.c,"debug-equipment") == 0)   {return lValNativeFunc(wwlnfDbgItem);}

	return lResolveNativeSymCommon(s);
}

static void cmdLisp(int c,const char *str, u8 id){
	static char reply[8192];
	memset(reply,0,sizeof(reply));
	lClosure *cl = lClosureNew(clRoot);
	setPID(cl,c);
	lVal *v = lEval(cl,lWrap(lRead(str)));
	lSPrintVal(v,reply,&reply[sizeof(reply)-1]);
	lClosureGC();

	if(id == 0){
		serverSendChatMsg(reply);
	}else{
		msgLispSExpr(c, id, reply);
	}
}

void lispInit(){
	lInit();
	clRoot = lClosureNew(NULL);
	setPID(clRoot,123);

	lispEvalNR("(define heal     (λ (a) (- (dmg (cond (a (- a)) (#t -20))))))");
	lispEvalNR("(define morning  (λ () (time  \"8:00\")))");
	lispEvalNR("(define night    (λ () (time \"23:00\")))");
	lispEvalNR("(define player-x (λ () (vx (player-pos))))");
	lispEvalNR("(define player-y (λ () (vy (player-pos))))");
	lispEvalNR("(define player-z (λ () (vz (player-pos))))");
	lispEvalNR("(define vx+      (λ (v o) (+ v (vec o 0 0))))");
	lispEvalNR("(define vy+      (λ (v o) (+ v (vec 0 o 0))))");
	lispEvalNR("(define vz+      (λ (v o) (+ v (vec 0 0 o))))");
	lispEvalNR("(define cloud-density (λ (a) (- 1.0 (/ (cloud-threshold (* (- 1.0 a) 256.0)) 256.0))))");
}

int parseCommand(uint c, const char *cmd){
	if(cmd[0] != '.'){return 0;}
	const char *tcmp = cmd+1;

	cmdLisp(c,tcmp,0);
	return 1;
}

void lispRecvSExpr(uint c,const packet *p){
	u8 id = p->v.u8[0];
	const char *str = (const char *)&p->v.u8[1];
	cmdLisp(c,str,id);
}
