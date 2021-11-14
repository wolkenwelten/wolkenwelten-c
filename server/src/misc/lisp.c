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
#include "../game/being.h"
#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/fire.h"
#include "../game/itemDrop.h"
#include "../game/weather.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../worldgen/worldgen.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/entity.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/game/time.h"
#include "../../../common/src/misc/line.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/misc/lisp.h"
#include "../../../common/src/network/messages.h"

#include "../../../common/nujel/lib/api.h"
#include "../../../common/nujel/lib/exception.h"
#include "../../../common/nujel/lib/s-expression/writer.h"
#include "../../../common/nujel/lib/allocation/roots.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

extern unsigned  int src_tmp_server_nuj_len;
extern unsigned char src_tmp_server_nuj_data[];

lSymbol *lsPID;

char replyBuf[256];

static uint getPID(lClosure *c){
	lVal *pid = lCar(lGetClosureSym(c, lsPID));
	if(pid == NULL){return 123;}
	return pid->vInt;
}

static void setPID(lClosure *c, uint pid){
	(void)c;
	lDefineVal(clRoot,"pid",lValInt(pid));
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
static lVal *wwlnfIDSlowCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(itemDropGetSlow());
}
static lVal *wwlnfIDFlush(lClosure *c, lVal *v){
	(void)c;(void)v;
	for(int i=itemDropCount-1;i>=0;i--){
		itemDropDel(i);
		addPriorityItemDrop(i);
	}
	return NULL;
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
	(void)c;
	const vec pos = castToVec(lCar(v),vecNOne());
	const int b = castToInt(lCadr(v),0);

	worldSetB(pos.x,pos.y,pos.z,b);
	return lValInt(b);
}

static lVal *wwlnfGetB(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec(lCar(v),vecNOne());

	return lValInt(worldGetB(pos.x,pos.y,pos.z));
}

static lVal *wwlnfTryB(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec(lCar(v),vecNOne());

	return lValInt(worldTryB(pos.x,pos.y,pos.z));
}

static lVal *wwlnfBox(lClosure *c, lVal *v){
	(void)c;
	const vec pos  = castToVec(lCar(v),vecNOne()); v = lCdr(v);
	const vec size = castToVec(lCar(v),vecZero()); v = lCdr(v);
	const int b    = castToInt(lCar(v),0);         v = lCdr(v);

	if((pos.x < 0) || (size.x <= 0)){return NULL;}
	worldBox(pos.x,pos.y,pos.z,size.x,size.y,size.z,b);
	return NULL;
}

static lVal *wwlnfMBox(lClosure *c, lVal *v){
	(void)c;
	const vec pos  = castToVec(lCar(v), vecNOne()); v = lCdr(v);
	const vec size = castToVec(lCar(v),vecZero()); v = lCdr(v);

	worldBoxMine(pos.x,pos.y,pos.z,size.x,size.y,size.z);
	return lValBool(true);
}

static lVal *wwlnfSphere(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec  (lCar(v),vecNOne()); v = lCdr(v);
	const float r = castToFloat(lCar(v),1.f);       v = lCdr(v);
	const int b   = castToInt  (lCar(v),0);

	worldBoxSphere(pos.x,pos.y,pos.z,r,b);
	return lValInt(b);
}

static lVal *wwlnfMSphere(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec  (lCar(v),vecNOne()); v = lCdr(v);
	const float r = castToFloat(lCar(v),4.f);

	worldBoxMineSphere(pos.x,pos.y,pos.z,r);
	return lValBool(true);
}

static lVal *wwlnfGive(lClosure *c, lVal *v){
	(void)c;
	const int id       = castToInt(lCar(v), -1); v = lCdr(v);
	const int amt      = castToInt(lCar(v),  0); v = lCdr(v);
	const int cplayer  = castToInt(lCar(v),getPID(c));

	msgPickupItem(cplayer,itemNew(id,amt));
	return NULL;
}

static lVal *wwlnfDmg(lClosure *c, lVal *v){
	const int hp      = castToInt(lCar(v),4); v = lCdr(v);
	const int cplayer = castToInt(lCar(v),getPID(c));

	msgBeingDamage(cplayer,hp,0,1.f,beingCharacter(cplayer),-1,vecZero());
	return NULL;
}

static lVal *wwlnfNewAnim(lClosure *c, lVal *v){
	(void)c;
	const vec pos    = castToVec(lCar(v),vecNOne()); v = lCdr(v);
	const int amount = castToInt(lCar(v),1);         v = lCdr(v);
	const int type   = castToInt(lCar(v),1);

	if(pos.x < 0){return NULL;}
	for(int i=0;i < amount;i++){
		animalNew(pos,type,-1);
	}
	return NULL;
}

static lVal *wwlnfSetAnim(lClosure *c, lVal *v){
	(void)c;
	const int index  = castToInt(lCar(v),-2); v = lCdr(v);
	const int hunger = castToInt(lCar(v),-2); v = lCdr(v);
	const int sleepy = castToInt(lCar(v),-2); v = lCdr(v);
	const int pregna = castToInt(lCar(v),-2); v = lCdr(v);
	const int state  = castToInt(lCar(v),-2); v = lCdr(v);
	const int health = castToInt(lCar(v),-2); v = lCdr(v);

	if((index < 0) || (index > (int)animalListMax)){return NULL;}
	animal *a = &animalList[index];
	if(a->type == 0){return NULL;}
	if(hunger >=  0){a->hunger    = hunger;}
	if(sleepy >=  0){a->sleepy    = sleepy;}
	if(pregna >= -1){a->pregnancy = pregna;}
	if(state  >=  0){a->state     = state;}
	if(health >=  0){a->health    = health;}
	return NULL;
}

static lVal *wwlnfTp(lClosure *c, lVal *v){
	vec pos     = castToVec(lCar(v),vecNOne()); v = lCdr(v);
	int cplayer = castToInt(lCar(v),getPID(c));

	if(!vecInWorld(pos)){return NULL;}
	const character *tpc = clients[cplayer].c;
	const vec rot = tpc == NULL ? vecZero() : tpc->rot;
	msgPlayerSetPos(cplayer, pos, rot, vecZero());
	return NULL;
}

static lVal *wwlnfTime(lClosure *c, lVal *v){
	(void)c;
	if(v != NULL){
		lVal *t = lCar(v);
		if(t != NULL){
			if(t->type == ltString){
				gtimeSetTimeOfDayHRS(t->vString->data);
			}else if(t->type == ltInt){
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
	for(uint i = animalListMax/2-1;i < animalListMax;i--){
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
	const uint target = castToInt(lCar(v),getPID(c));
	if(!getClientValid(target)){return NULL;}
	character *player = clients[target].c;
	if(player == NULL){return NULL;}

	characterEmptyInventory(player);
	characterUpdateItems(player);
	return NULL;
}

static lVal *wwlnfClearEq(lClosure *c, lVal *v){
	const uint target = castToInt(lCar(v),getPID(c));
	if(!getClientValid(target)){return NULL;}
	character *player = clients[target].c;
	if(player == NULL){return NULL;}

	characterEmptyEquipment(player);
	characterUpdateItems(player);
	return NULL;
}

static lVal *wwlnfSetInv(lClosure *c, lVal *v){
	const int slot   = castToInt(lCar(v),-1); v = lCdr(v);
	const int itemID = castToInt(lCar(v),-1); v = lCdr(v);
	const int amount = castToInt(lCar(v),1);  v = lCdr(v);
	const int target = castToInt(lCar(v),getPID(c));

	if((!getClientValid(target)) || (slot < 0)){return NULL;}
	if((itemID < 0) || (itemID > 4096)){return NULL;}
	character *player = clients[target].c;
	if(player == NULL){return NULL;}
	item itm = itemNew(itemID,amount);
	characterSetItemBarSlot(clients[target].c,slot,&itm);
	characterUpdateItems(player);

	return NULL;
}

static lVal *wwlnfSetEq(lClosure *c, lVal *v){
	const int slot   = castToInt(lCar(v),-1); v = lCdr(v);
	const int itemID = castToInt(lCar(v),-1); v = lCdr(v);
	const int amount = castToInt(lCar(v),1);  v = lCdr(v);
	const int target = castToInt(lCar(v),getPID(c));

	if((!getClientValid(target)) || (slot < 0)){return NULL;}
	character *player = clients[target].c;
	if(player == NULL){return NULL;}
	item itm = itemNew(itemID,amount);
	characterSetEquipmentSlot(clients[target].c,slot,&itm);
	characterUpdateItems(player);

	return NULL;
}

static lVal *wwlnfRCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(rainCount);
}

static lVal *wwlnfSendMessage(lClosure *c, lVal *v){
	(void)c;
	const char *msg = castToString(lCar(v),NULL);
	if(msg == NULL){return NULL;}
	serverSendChatMsg(msg);
	return lCar(v);
}

static lVal *wwlnfConsolePrint(lClosure *c, lVal *v){
	(void)c;
	const char *msg = castToString(lCar(v),NULL);
	if(msg == NULL){return NULL;}
	printf("%s\n",msg);
	return lCar(v);
}

static lVal *wwlnfQuit(lClosure *c, lVal *v){
	(void)c;(void)v;
	quit = true;
	return NULL;
}

static lVal *wwlnfChunkInfo(lClosure *c, lVal *v){
	(void)c;
	char buf[256];
	const vec pos = castToVec(lCar(v),vecNOne());
	if(pos.x < 0){return NULL;}

	chunk *chnk = worldTryChunk((uint)pos.x & 0xFFF0,(uint)pos.y & 0xFFF0,(uint)pos.z & 0xFFF0);
	if(chnk == NULL){return NULL;}
	snprintf(buf,sizeof(buf),"[%u:%u:%u] clientsUpdated: %x",chnk->x,chnk->y,chnk->z,chnk->clientsUpdated);

	return lValString(buf);
}

static lVal *wwlnfChungusInfo(lClosure *c, lVal *v){
	(void)c;
	char buf[256];
	const vec pos = castToVec(lCar(v),vecNOne());
	if(pos.x < 0){return NULL;}

	chungus *chng = worldTryChungus((uint)pos.x >> 8,(uint)pos.y >> 8,(uint)pos.z >> 8);
	if(chng == NULL){return NULL;}
	snprintf(buf,sizeof(buf),"[%u:%u:%u] clientsUpdated: %" PRIx64 " | clientsSubscribed: %" PRIx64 " | freeTimer: %" PRIu64 ,chng->x, chng->y, chng->z, chng->clientsUpdated,chng->clientsSubscribed,chng->freeTimer);

	return lValString(buf);
}

static lVal *wwlnfSetSpawnPos(lClosure *c, lVal *v){
	(void)c;
	const vec pos = castToVec(lCar(v),vecNOne());
	if(pos.x < 0){return NULL;}
	worldSetSpawnPos(pos);

	return NULL;
}

static lVal *wwlnfSpawnPos(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValVec(worldGetSpawnPos());
}

static lVal *wwlnfWorldgenSphere(lClosure *c, lVal *v){
	(void)c;
	const int x = castToInt(lCar(v),-1); v = lCdr(v);
	const int y = castToInt(lCar(v),-1); v = lCdr(v);
	const int z = castToInt(lCar(v),-1); v = lCdr(v);
	const int r = castToInt(lCar(v),16); v = lCdr(v);
	const int b = castToInt(lCar(v),I_Dirt);

	worldgenSphere(x,y,z,r,b);
	return NULL;
}

u8 wwlnfLineBlockSelection = 1;
void wwlnfLineSetBlock(int x, int y, int z){
	worldSetB(x,y,z,wwlnfLineBlockSelection);
}

static lVal *wwlnfLine(lClosure *c, lVal *v){
	(void)c;
	const vec p1 = castToVec(lCar(v),vecNOne()); v = lCdr(v);
	const vec p2 = castToVec(lCar(v),vecNOne()); v = lCdr(v);
	const int b =  castToInt(lCar(v),I_Dirt);

	wwlnfLineBlockSelection = b;
	if((p1.x < 0) || (p2.x < 0)){return NULL;}
	lineFromTo(p1.x,p1.y,p1.z,p2.x,p2.y,p2.z,wwlnfLineSetBlock);
	return NULL;
}

lVal *wwlnfAnimalKillAll(lClosure *c, lVal *v){
	(void)c; (void) v;
	animalDeleteAll();
	return NULL;
}

static lVal *wwlnfRResult(lClosure *c, lVal *v){
	(void)c;
	const int id     = castToInt(lCar(v),-1); v = lCdr(v);
	const int result = castToInt(lCar(v),0);  v = lCdr(v);
	const int amount = castToInt(lCar(v),0);

	if((id < 0) || (result <= 0) || (amount <= 0)){return NULL;}
	/*
	recipeCount = MAX(id+1,(int)recipeCount);
	recipes[id].result.ID = result;
	recipes[id].result.amount = amount;
	*/
	return NULL;
}

static lVal *wwlnfRIngred(lClosure *c, lVal *v){
	(void)c;
	const int id     = castToInt(lCar(v),-1); v = lCdr(v);
	const int ii     = castToInt(lCar(v),-1); v = lCdr(v);
	const int ingred = castToInt(lCar(v),0);  v = lCdr(v);
	const int amount = castToInt(lCar(v),0);

	if((id < 0) || (ii < 0) || (ii >= 4) || (ingred <= 0) || (amount <= 0)){return NULL;}
	/*
	recipeCount = MAX(id+1,(int)recipeCount);
	recipes[id].ingredient[ii].ID = ingred;
	recipes[id].ingredient[ii].amount = amount;
	*/
	return NULL;
}

void addServerNativeFuncs(lClosure *c){
	lAddNativeFunc(c,"player-pos",     "()",                                           "Returns player pos vector",                                  wwlnfPlayerPos);
	lAddNativeFunc(c,"animal-count",   "()",                                           "Returns animal count",                                       wwlnfACount);
	lAddNativeFunc(c,"animal-kill-all","()",                                           "Returns animal count",                                       wwlnfAnimalKillAll);
	lAddNativeFunc(c,"fire-count",     "()",                                           "Returns fire count",                                         wwlnfFCount);
	lAddNativeFunc(c,"mining-count",   "()",                                           "Returns block mining count",                                 wwlnfBMCount);
	lAddNativeFunc(c,"item-drop-count","()",                                           "Returns item drop count",                                    wwlnfIDCount);
	lAddNativeFunc(c,"item-drop-slow", "()",                                           "Returns amount of itemDrops that have the slow update bit",  wwlnfIDSlowCount);
	lAddNativeFunc(c,"item-drop-flush!","()",                                          "Remove all itemDrops",                                       wwlnfIDFlush);
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
	lAddNativeFunc(c,"line",           "(p1 p2 &b)",                                   "Set every block from P1 to P2 to &b=1",                      wwlnfLine);
	lAddNativeFunc(c,"sphere",         "(pos r &b)",                                   "Sets every block in the sphere at pos with radius r to &b=1",wwlnfSphere);
	lAddNativeFunc(c,"mbox",           "(pos size)",                                   "Mines every block in the box at pos with size",              wwlnfMBox);
	lAddNativeFunc(c,"msphere",        "(pos r)",                                      "Mines every block in the sphere at pos with radius r",       wwlnfMSphere);
	lAddNativeFunc(c,"game/time",      "(s)",                                          "Sets the time to the time string s",                         wwlnfTime);
	lAddNativeFunc(c,"tp",             "(pos)",                                        "Teleports to pos",                                           wwlnfTp);
	lAddNativeFunc(c,"send-message",   "(s)",                                          "Send a chat message to everyone",                            wwlnfSendMessage);
	lAddNativeFunc(c,"console-print",  "(s)",                                          "Prints something to stdout",                                 wwlnfConsolePrint);
	lAddNativeFunc(c,"chunk-info",     "(pos)",                                        "Returns a description of the chunk at pos",                  wwlnfChunkInfo);
	lAddNativeFunc(c,"chungus-info",   "(pos)",                                        "Returns a description of the chungus at pos",                wwlnfChungusInfo);
	lAddNativeFunc(c,"spawn-pos",      "()",                                           "Return the current spawn position as a vec",                 wwlnfSpawnPos);
	lAddNativeFunc(c,"spawn-pos!",     "(pos)",                                        "Set the spawn POS",                                          wwlnfSetSpawnPos);
	lAddNativeFunc(c,"worldgen/sphere","(x y z r b)",                                  "Only use during worldgen, sets a sphere of radius R at X Y Z to B",wwlnfWorldgenSphere);
	lAddNativeFunc(c,"quit!",          "()",                                           "Cleanly shuts down the server",                              wwlnfQuit);

	lAddNativeFunc(c,"r-result",       "(id result amt)",   "Set the result of recipe ID to AMT times RESULT.",       wwlnfRResult);
	lAddNativeFunc(c,"r-ingred",       "(id i ingred amt)", "Set the ingredient I of recipe ID to AMT times INGRED.", wwlnfRIngred);
}

static void *cmdLispReal(void *a, void *b){
	char reply[1<<16];
	memset(reply,0,sizeof(reply));

	u16 pid = *((uint *)a);
	const char *str = (const char *)b;

	lVal *expr = lRead(str);
	lVal *v = lnfDo(clients[pid].cl, expr);
	lSWriteVal(v,reply,&reply[sizeof(reply)-1],0,true);

	msgLispSExpr(pid, reply);

	return NULL;
}

static void cmdLisp(uint pid, const char *str){
	volatile uint pidp = pid;
	lExceptionTry(cmdLispReal, (void *)&pidp, (void *)str);
}

static void *lispInitReal(void *a, void *b){
	(void)a; (void)b;
	clRoot = lispCommonRoot(addServerNativeFuncs);
	lVal *expr = lRead((char *)src_tmp_server_nuj_data);
	lnfDo(clRoot, expr);
	lsPID = lSymS("pid");

	return NULL;
}

void lispInit(){
	lExceptionTry(lispInitReal,NULL,NULL);
}

lClosure *lispClientClosure(uint pid){
	lClosure *ret = lClosureNew(clRoot);
	lRootsClosurePush(ret);
	setPID(ret,pid);
	return ret;
}

int parseCommand(uint pid, const char *cmd){
	if(cmd[0] != '.'){return 0;}
	const char *tcmp = cmd+1;

	cmdLisp(pid,tcmp);
	return 1;
}

void lispRecvSExpr(uint pid,const packet *p){
	printf("Recv: %s\n",(const char *)p->v.u8);
	cmdLisp(pid,(const char *)p->v.u8);
}

void lispEvents(){
	static lVal *yieldRun = NULL;
	static u64 lastTicks = 0;
	PROFILE_START();

	const u64 cticks = getTicks();
	if((lastTicks + 50) > cticks){return;}
	lastTicks = cticks;
	if(yieldRun == NULL){
		yieldRun = lRootsValPush(lCons(NULL,NULL));
		yieldRun->vList.car = lValSym("yield-run");
	}
	lEval(clRoot,yieldRun);

	PROFILE_STOP();
}

void *lispEvalReal(void *a, void *b){
	static char reply[1<<12];
	const char *str = a;
	bool humanReadable = b != NULL;
	memset(reply,0,sizeof(reply));

	lVal *expr = lRead(str);
	lVal *v = lnfDo(clRoot,expr);
	lSWriteVal(v,reply,&reply[sizeof(reply)-1],0,humanReadable);

	return reply;
}

const char *lispEval(const char *str, bool humanReadable){
	return lExceptionTry(lispEvalReal, (void *)str, humanReadable ? (void *)str : NULL);
}

void lGUIWidgetFree(lVal *v){
	(void)v;
}
