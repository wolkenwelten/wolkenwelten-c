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

#include "lisp.h"

#include "../main.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/entity.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/time.h"
#include "../game/rain.h"
#include "../game/weather.h"
#include "../network/server.h"
#include "../tmp/assets.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/misc/lisp.h"
#include "../../../common/src/network/messages.h"

#include "../../../common/src/nujel/nujel.h"
#include "../../../common/src/nujel/casting.h"
#include "../../../common/src/nujel/reader.h"
#include "../../../common/src/nujel/string.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

char replyBuf[256];

static uint getPID(lClosure *c){
	lVal *pid = lResolveSym(c, lValSym("pid"));
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

static lVal *wwlnfACount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(animalCount);
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

static lVal *wwlnfPlayerPos(lClosure *c, lVal *v){
	(void)v;
	const int cc = getPID(c);
	if((cc < 0) || (cc >= 32)){return NULL;}
	if(clients[cc].state)     {return NULL;}
	if(clients[cc].c == NULL) {return NULL;}
	return lValVec(clients[cc].c->pos);
}

static lVal *wwlnfSetB(lClosure *c, lVal *v){
	vec pos = vecNOne();
	u8 b = 0;

	for(int i=0;i<2;i++){
		if(v == NULL){return NULL;}
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

static lVal *wwlnfTryB(lClosure *c, lVal *v){
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return lValInt(0);}
	t = lnfVec(c,v);
	if(t == NULL){return lValInt(0);}
	return lValInt(worldTryB(t->vVec.x,t->vVec.y,t->vVec.z));
}

static lVal *wwlnfBox(lClosure *c, lVal *v){
	vec arg[2] = {vecZero(),vecOne()};
	u8 b = 0;

	for(int i=0;i<3;i++){
		if(v == NULL){return NULL;}
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
		if(v == NULL){return NULL;}
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
		if(v == NULL){return NULL;}
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
		if(v == NULL){return NULL;}
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
				return NULL;
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
	if(type <= 0){return NULL;}
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

static lVal *wwlnfClearInv(lClosure *c, lVal *v){
	item newInventory[40];
	memset(newInventory,0,sizeof(newInventory));

	int target = getPID(c);
	for(int i=0;i<1;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if((t == NULL) && (t->type != ltInt)){break;}
		target= t->vInt;
	}
	if(!getClientValid(target)){return lValBool(false);}
	msgPlayerSetInventory(target,newInventory,40);
	return lValBool(true);
}

static lVal *wwlnfSetInv(lClosure *c, lVal *v){
	int args[4] = {-1, -1, 1, getPID(c)};

	for(int i=0;i<4;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if((t == NULL) && (t->type != ltInt)){break;}
		args[i] = t->vInt;
	}
	if((!getClientValid(args[3])) || (args[0] < 0) || (args[0] >= 40) || (args[0] < 0)){return lValBool(false);}
	clients[args[3]].c->inventory[args[0]] = itemNew(args[1],args[2]);
	msgPlayerSetInventory(args[3],clients[args[3]].c->inventory,40);
	return lValBool(true);
}

static lVal *wwlnfSetEq(lClosure *c, lVal *v){
	int args[4] = {-1, -1, 1, getPID(c)};

	for(int i=0;i<4;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if((t == NULL) && (t->type != ltInt)){break;}
		args[i] = t->vInt;
	}
	if((!getClientValid(args[3])) || (args[0] < 0) || (args[0] >= 3) || (args[0] < 0)){return lValBool(false);}
	clients[args[3]].c->equipment[args[0]] = itemNew(args[1],args[2]);
	msgPlayerSetEquipment(args[3],clients[args[3]].c->equipment,3);
	return lValBool(true);
}

static lVal *wwlnfClearEq(lClosure *c, lVal *v){
	item newEquipment[3];
	memset(newEquipment,0,sizeof(newEquipment));

	int target = getPID(c);
	for(int i=0;i<1;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if((t == NULL) && (t->type != ltInt)){continue;}
		target = t->vInt;
	}
	if(!getClientValid(target)){return lValBool(false);}
	msgPlayerSetEquipment(target,newEquipment, 3);
	return lValBool(true);
}

static lVal *wwlnfRCount(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValInt(rainCount);
}

static lVal *wwlnfSendMessage(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	serverSendChatMsg(t->vString->data);
	return t;
}

static lVal *wwlnfConsolePrint(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	printf("%s\n",t->vString->data);
	return t;
}

static lVal *wwlnfQuit(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	quit = true;
	return NULL;
}

static lVal *wwlnfChunkInfo(lClosure *c, lVal *v){
	char buf[256];

	if(v == NULL){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lnfVec(c,t);
	vec pos = t->vVec;

	chunk *chnk = worldTryChunk((uint)pos.x & 0xFFF0,(uint)pos.y & 0xFFF0,(uint)pos.z & 0xFFF0);
	if(chnk == NULL){return NULL;}
	snprintf(buf,sizeof(buf),"[%u:%u:%u] clientsUpdated: %x",chnk->x,chnk->y,chnk->z,chnk->clientsUpdated);

	return lValString(buf);
}

static lVal *wwlnfChungusInfo(lClosure *c, lVal *v){
	char buf[256];

	if(v == NULL){return NULL;}
	lVal *t = lEval(c,v->vList.car);
	if(t == NULL){return NULL;}
	t = lnfVec(c,t);
	vec pos = t->vVec;

	chungus *chng = worldTryChungus((uint)pos.x >> 8,(uint)pos.y >> 8,(uint)pos.z >> 8);
	if(chng == NULL){return NULL;}
	snprintf(buf,sizeof(buf),"[%u:%u:%u] clientsUpdated: %" PRIx64 " | clientsSubscribed: %" PRIx64 " | freeTimer: %" PRIu64 ,chng->x, chng->y, chng->z, chng->clientsUpdated,chng->clientsSubscribed,chng->freeTimer);

	return lValString(buf);
}

void addServerNativeFuncs(lClosure *c){
	lAddNativeFunc(c,"player-pos",     "()",                                           "Returns player pos vector",                                  wwlnfPlayerPos);
	lAddNativeFunc(c,"animal-count",   "()",                                           "Returns animal count",                                       wwlnfACount);
	lAddNativeFunc(c,"fire-count",     "()",                                           "Returns fire count",                                         wwlnfFCount);
	lAddNativeFunc(c,"mining-count",   "()",                                           "Returns block mining count",                                 wwlnfBMCount);
	lAddNativeFunc(c,"item-drop-count","()",                                           "Returns item drop count",                                    wwlnfIDCount);
	lAddNativeFunc(c,"entity-count",   "()",                                           "Returns entity count",                                       wwlnfECount);
	lAddNativeFunc(c,"chungus-count",  "()",                                           "Returns chungus count",                                      wwlnfChungi);
	lAddNativeFunc(c,"rain-count",     "()",                                           "Returns amount of rain drops",                               wwlnfRCount);
	lAddNativeFunc(c,"load-shed!",     "()",                                           "Load shedding, mostly unloading chungi",                     wwlnfLShed);
	lAddNativeFunc(c,"give!",          "(id &amount &player)",                         "Gives &player=pid &amount=1 of item id",                     wwlnfGive);
	lAddNativeFunc(c,"clear-inv!",     "(&player)",                                    "Clears the inventory of &player=pid",                        wwlnfClearInv);
	lAddNativeFunc(c,"set-inv!",       "(slot id &amount &player)",                    "Sets inventory slot of &player=pid to &amount=1 id items",   wwlnfSetInv);
	lAddNativeFunc(c,"clear-eq!",      "(&player)",                                    "Clears the equipment of &player=pid",                        wwlnfClearEq);
	lAddNativeFunc(c,"set-eq!",        "(slot id &amount &player)",                    "Sets inventory slot of &player=pid to &amount=1 id items",   wwlnfSetEq);
	lAddNativeFunc(c,"dmg!",           "(&amount &player)",                            "Damages &player=pid by &amount=4 points",                    wwlnfDmg);
	lAddNativeFunc(c,"die!",           "(&player)",                                    "Kills &player=pid immediatly",                               wwlnfDie);
	lAddNativeFunc(c,"animal-new",     "(pos &type &amount)",                          "Creates &amount=1 new animals of &type=1 at pos",            wwlnfNewAnim);
	lAddNativeFunc(c,"animal-set",     "(i &hunger &sleepy &pregnancy &state &health)","Sets the fields for animal i",                               wwlnfSetAnim);
	lAddNativeFunc(c,"cloud-thresh!",  "(a)",                                          "Sets cloud threshold to a",                                  wwlnfCDen);
	lAddNativeFunc(c,"wind-velocity",  "(v)",                                          "Sets wind velocity to vector v",                             wwlnfWVel);
	lAddNativeFunc(c,"rain-set",       "(a)",                                          "Sets rain rate to a",                                        wwlnfRain);
	lAddNativeFunc(c,"setb!",          "(pos b)",                                      "Sets block at pos to b",                                     wwlnfSetB);
	lAddNativeFunc(c,"tryb",           "(pos)",                                        "Tries and gets block type at pos",                           wwlnfTryB);
	lAddNativeFunc(c,"getb!",          "(pos)",                                        "Gets block type at pos, might trigger worldgen",             wwlnfGetB);
	lAddNativeFunc(c,"box",            "(pos size &b)",                                "Sets every block in the box at pos with size to &b=1",       wwlnfBox);
	lAddNativeFunc(c,"sphere",         "(pos r &b)",                                   "Sets every block in the sphere at pos with radius r to &b=1",wwlnfSphere);
	lAddNativeFunc(c,"mbox",           "(pos size)",                                   "Mines every block in the box at pos with size",              wwlnfMBox);
	lAddNativeFunc(c,"msphere",        "(pos r)",                                      "Mines every block in the sphere at pos with radius r",       wwlnfMSphere);
	lAddNativeFunc(c,"time",           "(s)",                                          "Sets the time to the time string s",                         wwlnfTime);
	lAddNativeFunc(c,"tp",             "(pos)",                                        "Teleports to pos",                                           wwlnfTp);
	lAddNativeFunc(c,"send-message",   "(s)",                                          "Send a chat message to everyone",                            wwlnfSendMessage);
	lAddNativeFunc(c,"console-print",  "(s)",                                          "Prints something to stdout",                                 wwlnfConsolePrint);
	lAddNativeFunc(c,"chunk-info",     "(pos)",                                        "Returns a description of the chunk at pos",                  wwlnfChunkInfo);
	lAddNativeFunc(c,"chungus-info",   "(pos)",                                        "Returns a description of the chungus at pos",                wwlnfChungusInfo);

	lAddNativeFunc(c,"quit!",          "()",                                           "Cleanly shuts down the server",                              wwlnfQuit);
}

static void cmdLisp(int c,const char *str, u8 id){
	static char reply[8192];
	memset(reply,0,sizeof(reply));

	lVal *v = lEval(clients[c].cl,lWrap(lRead(str)));
	lSDisplayVal(v,reply,&reply[sizeof(reply)-1]);
	lClosureGC();

	if(id == 0){
		serverSendChatMsg(reply);
	}else{
		msgLispSExpr(c, id, reply);
	}
}

void lispInit(){
	lInit();

	clRoot = lispCommonRoot();
	addServerNativeFuncs(clRoot);
	lEval(clRoot,lWrap(lRead((char *)src_tmp_server_nuj_data)));

	lClosureGC();
}

lClosure *lispClientClosure(uint c){
	lClosure *ret = lClosureNew(clRoot);
	ret->flags |= lfNoGC;
	setPID(ret,c);
	return ret;
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

void lispEvents(){
	static u64 lastTicks = 0;
	static lVal *expr = NULL;
	if(expr == NULL){
		expr = lWrap(lRead("(yield-run)"));
		expr->flags |= lfNoGC;
	}

	PROFILE_START();

	const u64 cticks = getTicks();
	if((lastTicks + 500) > cticks){return;}
	lastTicks = cticks;

	lEval(clRoot,expr);
	lClosureGC();

	PROFILE_STOP();
}

const char *lispEval(const char *str){
	static char reply[4096];
	memset(reply,0,sizeof(reply));
	lVal *v = lEval(clRoot,lWrap(lRead(str)));
	lSDisplayVal(v,reply,&reply[sizeof(reply)-1]);

	int soff,slen,len = strnlen(reply,sizeof(reply)-1);
	for(soff = 0;    isspace((u8)reply[soff]) || (reply[soff] == '"');soff++){}
	for(slen = len-1;isspace((u8)reply[slen]) || (reply[slen] == '"');slen--){reply[slen] = 0;}

	lClosureGC();
	return reply+soff;
}
