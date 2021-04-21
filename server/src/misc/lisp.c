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
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/weather.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/entity.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/game/time.h"
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

extern        size_t src_tmp_server_nuj_len;
extern unsigned char src_tmp_server_nuj_data[];

char replyBuf[256];

static uint getPID(lClosure *c){
	lVal *pid = lResolveSym(c - lClosureList, lValSym("pid"));
	if(pid == NULL){return 123;}
	return pid->vInt;
}

static void setPID(lClosure *c, uint pid){
	lVal *sym = lValSym("pid");
	lVal *t = lDefineClosureSym(c - lClosureList, lvSym(sym->vCdr));
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
	int b   = 0;

	v = getLArgV(c,v,&pos);
	v = getLArgI(c,v,&b);

	worldSetB(pos.x,pos.y,pos.z,b);
	return lValInt(b);
}

static lVal *wwlnfGetB(lClosure *c, lVal *v){
	vec pos = vecNOne();

	v = getLArgV(c,v,&pos);

	return lValInt(worldGetB(pos.x,pos.y,pos.z));
}

static lVal *wwlnfTryB(lClosure *c, lVal *v){
	vec pos = vecNOne();

	v = getLArgV(c,v,&pos);

	return lValInt(worldTryB(pos.x,pos.y,pos.z));
}

static lVal *wwlnfBox(lClosure *c, lVal *v){
	vec pos  = vecNOne();
	vec size = vecZero();
	int b    = 0;

	v = getLArgV(c,v,&pos);
	v = getLArgV(c,v,&size);
	v = getLArgI(c,v,&b);

	if((pos.x < 0) || (size.x <= 0)){return NULL;}
	worldBox(pos.x,pos.y,pos.z,size.x,size.y,size.z,b);
	return NULL;
}

static lVal *wwlnfMBox(lClosure *c, lVal *v){
	vec pos  = vecNOne();
	vec size = vecZero();

	v = getLArgV(c,v,&pos);
	v = getLArgV(c,v,&size);

	worldBoxMine(pos.x,pos.y,pos.z,size.x,size.y,size.z);
	return lValBool(true);
}

static lVal *wwlnfSphere(lClosure *c, lVal *v){
	vec pos = vecNOne();
	float r = 1.f;
	int b   = 0;

	v = getLArgV(c,v,&pos);
	v = getLArgF(c,v,&r);
	v = getLArgI(c,v,&b);

	worldBoxSphere(pos.x,pos.y,pos.z,r,b);
	return lValInt(b);
}

static lVal *wwlnfMSphere(lClosure *c, lVal *v){
	vec pos = vecNOne();
	float r = 4.f;

	v = getLArgV(c,v,&pos);
	v = getLArgF(c,v,&r);

	worldBoxMineSphere(pos.x,pos.y,pos.z,r);
	return lValBool(true);
}

static lVal *wwlnfGive(lClosure *c, lVal *v){
	int id       = -1;
	int amt      =  0;
	int cplayer  = -1;

	v = getLArgI(c,v,&id);
	v = getLArgI(c,v,&amt);
	v = getLArgI(c,v,&cplayer);
	if(cplayer < 0){cplayer = getPID(c);}

	msgPickupItem(cplayer,itemNew(id,amt));
	return NULL;
}

static lVal *wwlnfDmg(lClosure *c, lVal *v){
	int hp      =  4;
	int cplayer = -1;

	v = getLArgI(c,v,&hp);
	v = getLArgI(c,v,&cplayer);
	if(cplayer < 0){cplayer = getPID(c);}

	msgBeingDamage(cplayer,hp,0,1.f,beingCharacter(cplayer),-1,vecZero());
	return NULL;
}

static lVal *wwlnfNewAnim(lClosure *c, lVal *v){
	vec pos    = vecNOne();
	int amount = 1;
	int type   = 1;

	v = getLArgV(c,v,&pos);
	v = getLArgI(c,v,&amount);
	v = getLArgI(c,v,&type);

	if(pos.x < 0){return NULL;}
	for(int i=0;i < amount;i++){
		animalNew(pos,type,-1);
	}
	return NULL;
}

static lVal *wwlnfSetAnim(lClosure *c, lVal *v){
	int index  = -1;
	int hunger = -1;
	int sleepy = -1;
	int pregna = -2;
	int state  = -1;
	int health = -1;

	v = getLArgI(c,v,&index);
	v = getLArgI(c,v,&hunger);
	v = getLArgI(c,v,&sleepy);
	v = getLArgI(c,v,&pregna);
	v = getLArgI(c,v,&state);
	v = getLArgI(c,v,&health);

	if((index < 0) || (index > (int)animalCount)){return NULL;}
	animal *a = &animalList[index];
	if(a->type ==  0){return NULL;}
	if(hunger >=  0){a->hunger    = hunger;}
	if(sleepy >=  0){a->sleepy    = sleepy;}
	if(pregna >= -1){a->pregnancy = pregna;}
	if(state  >=  0){a->state     = state;}
	if(health >=  0){a->health    = health;}
	return NULL;
}

static lVal *wwlnfTp(lClosure *c, lVal *v){
	vec pos     = vecNOne();
	int cplayer = -1;

	v = getLArgV(c,v,&pos);
	v = getLArgI(c,v,&cplayer);

	if(cplayer < 0){cplayer = getPID(c);}
	if(!vecInWorld(pos)){return NULL;}
	const character *tpc = clients[cplayer].c;
	const vec rot = tpc == NULL ? vecZero() : tpc->rot;
	msgPlayerSetPos(cplayer, pos, rot, vecZero());
	return NULL;
}

static lVal *wwlnfTime(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lEval(c,lCar(v));
		if(t != NULL){
			if(t->type == ltString){
				gtimeSetTimeOfDayHRS(lStrData(t));
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
	(void)c;(void)v;

	chungusFreeOldChungi(100);
	for(uint i = animalCount/2-1;i < animalCount;i--){
		if(animalList[i].type == 0){continue;}
		animalDel(i);
	}

	return NULL;
}

static lVal *wwlnfChungi(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(chungusCount - chungusFreeCount);
}

static lVal *wwlnfClearInv(lClosure *c, lVal *v){
	item newInventory[40];
	memset(newInventory,0,sizeof(newInventory));

	int target = getPID(c);
	for(int i=0;i<1;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,lCar(v));
		v = lCdr(v);
		if((t == NULL) && (t->type != ltInt)){break;}
		target= t->vInt;
	}
	if(!getClientValid(target)){return NULL;}
	msgPlayerSetInventory(target,newInventory,40);
	return NULL;
}

static lVal *wwlnfSetInv(lClosure *c, lVal *v){
	int args[4] = {-1, -1, 1, getPID(c)};

	for(int i=0;i<4;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,lCar(v));
		v = lCdr(v);
		if((t == NULL) || (t->type != ltInt)){break;}
		args[i] = t->vInt;
	}
	if((!getClientValid(args[3])) || (args[0] < 0) || (args[0] >= 40) || (args[0] < 0)){return NULL;}
	clients[args[3]].c->inventory[args[0]] = itemNew(args[1],args[2]);
	msgPlayerSetInventory(args[3],clients[args[3]].c->inventory,40);
	return NULL;
}

static lVal *wwlnfSetEq(lClosure *c, lVal *v){
	int args[4] = {-1, -1, 1, getPID(c)};

	for(int i=0;i<4;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,lCar(v));
		v = lCdr(v);
		if((t == NULL) && (t->type != ltInt)){break;}
		args[i] = t->vInt;
	}
	if((!getClientValid(args[3])) || (args[0] < 0) || (args[0] >= 3) || (args[0] < 0)){return NULL;}
	clients[args[3]].c->equipment[args[0]] = itemNew(args[1],args[2]);
	msgPlayerSetEquipment(args[3],clients[args[3]].c->equipment,3);
	return NULL;
}

static lVal *wwlnfClearEq(lClosure *c, lVal *v){
	item newEquipment[3];
	memset(newEquipment,0,sizeof(newEquipment));

	int target = getPID(c);
	for(int i=0;i<1;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,lCar(v));
		v = lCdr(v);
		if((t == NULL) && (t->type != ltInt)){continue;}
		target = t->vInt;
	}
	if(!getClientValid(target)){return NULL;}
	msgPlayerSetEquipment(target,newEquipment, 3);
	return NULL;
}

static lVal *wwlnfRCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(rainCount);
}

static lVal *wwlnfSendMessage(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	serverSendChatMsg(lStrData(t));
	return t;
}

static lVal *wwlnfConsolePrint(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	printf("%s\n",lStrData(t));
	return t;
}

static lVal *wwlnfQuit(lClosure *c, lVal *v){
	(void)c;(void)v;
	quit = true;
	return NULL;
}

static lVal *wwlnfChunkInfo(lClosure *c, lVal *v){
	char buf[256];
	vec pos = vecNOne();

	v = getLArgV(c,v,&pos);
	if(pos.x < 0){return NULL;}

	chunk *chnk = worldTryChunk((uint)pos.x & 0xFFF0,(uint)pos.y & 0xFFF0,(uint)pos.z & 0xFFF0);
	if(chnk == NULL){return NULL;}
	snprintf(buf,sizeof(buf),"[%u:%u:%u] clientsUpdated: %x",chnk->x,chnk->y,chnk->z,chnk->clientsUpdated);

	return lValString(buf);
}

static lVal *wwlnfChungusInfo(lClosure *c, lVal *v){
	char buf[256];
	vec pos = vecNOne();
	v=  getLArgV(c,v,&pos);
	if(pos.x < 0){return NULL;}

	chungus *chng = worldTryChungus((uint)pos.x >> 8,(uint)pos.y >> 8,(uint)pos.z >> 8);
	if(chng == NULL){return NULL;}
	snprintf(buf,sizeof(buf),"[%u:%u:%u] clientsUpdated: %" PRIx64 " | clientsSubscribed: %" PRIx64 " | freeTimer: %" PRIu64 ,chng->x, chng->y, chng->z, chng->clientsUpdated,chng->clientsSubscribed,chng->freeTimer);

	return lValString(buf);
}

static lVal *wwlnfSetSpawnPos(lClosure *c, lVal *v){
	vec pos = vecNOne();

	v =  getLArgV(c,v,&pos);
	if(pos.x < 0){return NULL;}
	worldSetSpawnPos(ivecNewV(pos));

	return NULL;
}

static lVal *wwlnfSpawnPos(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValVec(vecNewI(worldGetSpawnPos()));
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
	lAddNativeFunc(c,"player-dmg",     "(&amount &player)",                            "Damages &player=pid by &amount=4 points",                    wwlnfDmg);
	lAddNativeFunc(c,"animal-new",     "(pos &type &amount)",                          "Creates &amount=1 new animals of &type=1 at pos",            wwlnfNewAnim);
	lAddNativeFunc(c,"animal-set",     "(i &hunger &sleepy &pregnancy &state &health)","Sets the fields for animal i",                               wwlnfSetAnim);
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
	lAddNativeFunc(c,"spawn-pos",      "()",                                           "Return the current spawn position as a vec",                 wwlnfSpawnPos);
	lAddNativeFunc(c,"spawn-pos!",     "(pos)",                                        "Set the spawn POS",                                          wwlnfSetSpawnPos);

	lAddNativeFunc(c,"quit!",          "()",                                           "Cleanly shuts down the server",                              wwlnfQuit);
}

static void cmdLisp(int c,const char *str){
	static char reply[8192];
	memset(reply,0,sizeof(reply));

	lVal *v = lEval(clients[c].cl,lWrap(lRead(str)));
	lSWriteVal(v,reply,&reply[sizeof(reply)-1],0,true);
	lClosureGC();

	msgLispSExpr(c, reply);
}

void lispInit(){
	lInit();

	clRoot = lispCommonRoot();
	addServerNativeFuncs(clRoot);
	lEval(clRoot,lWrap(lRead((char *)src_tmp_server_nuj_data)));

	lClosureGC();
}

lClosure *lispClientClosure(uint c){
	const uint i = lClosureNew(clRoot - lClosureList);
	lClosure *ret = &lClosureList[i];
	ret->flags |= lfNoGC;
	setPID(ret,c);
	return ret;
}

int parseCommand(uint c, const char *cmd){
	if(cmd[0] != '.'){return 0;}
	const char *tcmp = cmd+1;

	cmdLisp(c,tcmp);
	return 1;
}

void lispRecvSExpr(uint c,const packet *p){
	cmdLisp(c,(const char *)p->v.u8);
}

void lispEvents(){
	static lVal *yieldRun = NULL;
	static u64 lastTicks = 0;
	PROFILE_START();

	const u64 cticks = getTicks();
	if((lastTicks + 50) > cticks){return;}
	lastTicks = cticks;
	if(yieldRun == NULL){
		yieldRun = lCons(lValSym("yield-run"),NULL);
	}

	yieldRun->flags |= lfNoGC;

	lEval(clRoot,yieldRun);
	lClosureGC();

	PROFILE_STOP();
}

const char *lispEval(const char *str){
	static char reply[4096];
	memset(reply,0,sizeof(reply));
	lVal *v = lEval(clRoot,lWrap(lRead(str)));
	lSWriteVal(v,reply,&reply[sizeof(reply)-1],0,true);

	int soff,slen,len = strnlen(reply,sizeof(reply)-1);
	for(soff = 0;    isspace((u8)reply[soff]) || (reply[soff] == '"');soff++){}
	for(slen = len-1;isspace((u8)reply[slen]) || (reply[slen] == '"');slen--){reply[slen] = 0;}

	lClosureGC();
	return reply+soff;
}

void lGUIWidgetFree(lVal *v){
	(void)v;
}
