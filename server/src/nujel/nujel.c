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
#include "nujel.h"

#include "../main.h"
#include "../game/being.h"
#include "../game/beamblast.h"
#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/fire.h"
#include "../game/weather/weather.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"
#include "../worldgen/worldgen.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/entity.h"
#include "../../../common/src/game/time.h"
#include "../../../common/src/misc/line.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/nujel/nujel.h"
#include "../../../common/src/network/messages.h"

#include "../../../common/nujel/lib/api.h"
#include "../../../common/nujel/lib/exception.h"
#include "../../../common/nujel/lib/misc/pf.h"
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

static uint getPID(lClosure *c){
	lVal *pid = lCar(lGetClosureSym(c, lsPID));
	if(pid == NULL){return 123;}
	return pid->vInt;
}

void lPrintError(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
}

static lVal *wwlnfBMCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(blockMiningGetActive());
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
	return NULL;
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

static lVal *wwlnfDmg(lClosure *c, lVal *v){
	const int hp      = castToInt(lCar(v),4); v = lCdr(v);
	const int cplayer = castToInt(lCar(v),getPID(c));

	msgBeingDamage(cplayer,hp,0,1.f,beingCharacter(cplayer),-1,vecZero());
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

static lVal *wwlnfChungi(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(chungusCount - chungusFreeCount);
}

static lVal *wwlnfRCount(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(rainCount);
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
	const int b = castToInt(lCar(v), 1);

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
	const int b =  castToInt(lCar(v), 1);

	wwlnfLineBlockSelection = b;
	if((p1.x < 0) || (p2.x < 0)){return NULL;}
	lineFromTo(p1.x,p1.y,p1.z,p2.x,p2.y,p2.z,wwlnfLineSetBlock);
	return NULL;
}

static lVal *wwlnfExplode(lClosure *c, lVal *v){
	const vec pos   = requireVec(c, lCar(v));
	const float pow = requireFloat(c, lCadr(v));
	const int style = requireInt(c, lCaddr(v));
	explode(pos,pow,style);
	return lCar(v);
}

static lVal *wwlnfCharacterBeing(lClosure *c, lVal *v){
	const int clientID = requireInt(c, lCar(v));
	if((clientID < 0) || (clientID > (int)countof(clients)) || (clients[clientID].state)){
		lExceptionThrowValClo("invalid-reference", "Can't turn that into a player being", lCar(v), c);
	}
	return lValInt(characterGetBeing(clients[clientID].c));
}

void addServerNativeFuncs(lClosure *c){
	lAddNativeFunc(c,"character->being", "[player]",                                     "Returns PLAYER as a being",                                wwlnfCharacterBeing);
	lAddNativeFunc(c,"player-pos",     "()",                                           "Returns player pos vector",                                  wwlnfPlayerPos);
	lAddNativeFunc(c,"mining-count",   "()",                                           "Returns block mining count",                                 wwlnfBMCount);
	lAddNativeFunc(c,"entity-count",   "()",                                           "Returns entity count",                                       wwlnfECount);
	lAddNativeFunc(c,"chungus-count",  "()",                                           "Returns chungus count",                                      wwlnfChungi);
	lAddNativeFunc(c,"rain-count",     "()",                                           "Returns amount of rain drops",                               wwlnfRCount);
	lAddNativeFunc(c,"player-dmg",     "(&amount &player)",                            "Damages &player=pid by &amount=4 points",                    wwlnfDmg);
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
	lAddNativeFunc(c,"chunk-info",     "(pos)",                                        "Returns a description of the chunk at pos",                  wwlnfChunkInfo);
	lAddNativeFunc(c,"chungus-info",   "(pos)",                                        "Returns a description of the chungus at pos",                wwlnfChungusInfo);
	lAddNativeFunc(c,"spawn-pos",      "()",                                           "Return the current spawn position as a vec",                 wwlnfSpawnPos);
	lAddNativeFunc(c,"spawn-pos!",     "(pos)",                                        "Set the spawn POS",                                          wwlnfSetSpawnPos);
	lAddNativeFunc(c,"worldgen/sphere","(x y z r b)",                                  "Only use during worldgen, sets a sphere of radius R at X Y Z to B",wwlnfWorldgenSphere);
	lAddNativeFunc(c,"quit!",          "()",                                           "Cleanly shuts down the server",                              wwlnfQuit);
	lAddNativeFunc(c,"explode!",       "[pos pow style]",                              "Create a new explision at POS with POW and STYLE",           wwlnfExplode);
}

static void *lispInitReal(void *a, void *b){
	(void)a; (void)b;
	clRoot = lispCommonRoot(addServerNativeFuncs);
	lLoadS(clRoot,(const char *)src_tmp_server_nuj_data, src_tmp_server_nuj_len);
	lsPID = RSYMP(lSymS("pid"));

	return NULL;
}

void lispInit(){
	lExceptionTryExit(lispInitReal,NULL,NULL);
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

void lGUIWidgetFree(lVal *v){
	(void)v;
}