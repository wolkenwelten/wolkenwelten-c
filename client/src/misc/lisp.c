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
#include "../game/beamblast.h"
#include "../game/character.h"
#include "../game/fire.h"
#include "../game/grenade.h"
#include "../game/projectile.h"
#include "../game/recipe.h"
#include "../game/throwable.h"
#include "../game/weather.h"
#include "../gfx/gfx.h"
#include "../gfx/texture.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/lispInput.h"
#include "../gui/textInput.h"
#include "../misc/options.h"
#include "../network/chat.h"
#include "../network/client.h"
#include "../sdl/sdl.h"
#include "../sdl/sfx.h"
#include "../tmp/assets.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/lisp.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"
#include "../../../common/src/nujel/nujel.h"
#include "../../../common/src/nujel/casting.h"
#include "../../../common/src/nujel/reader.h"
#include "../../../common/src/nujel/string.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

u8 SEvalID;

void lispKeyDown(int code){
	lVal *expr;
	lVal *lCode = lValInt(code);
	lVal *lArrRef = lCons(lValSym("arr-ref"),lCons(lValSym("input-keydown"),lCons(lCode,NULL)));
	expr = lCons(lArrRef,lCons(lCode,NULL));
	lEval(clRoot,expr);
}

void lPrintError(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
}

lVal *lispSEvalSym(u8 id){
	static char buf[8];
	snprintf(buf,sizeof(buf),"SEv%03u",id);
	return lValSym(buf);
}

lVal *lispEvalNR(const char *str){
	return lEval(clRoot,lWrap(lRead(str)));
}

static lVal *wwlnfSEval(lClosure *c, lVal *v){
	(void)c;
	static char buf[8192];
	memset(buf,0,sizeof(buf));
	lSWriteVal(lWrap(v),buf,&buf[sizeof(buf)-1]);
	if(++SEvalID == 0){++SEvalID;}
	msgLispSExpr(-1,SEvalID,buf);
	return lispSEvalSym(SEvalID);
}

static lVal *wwlnfPlayerName(lClosure *c, lVal *v){
	const char *npName = NULL;

	getLArgS(npName);

	if(npName != NULL){
		snprintf(playerName,sizeof(playerName),"%s",npName);
	}
	return lValString(playerName);
}

static lVal *wwlnfSoundVolume(lClosure *c, lVal *v){
	float nvol = -1.f;
	getLArgF(nvol);
	if(nvol > -0.1f){optionSoundVolume = nvol;}
	return lValFloat(optionSoundVolume);
}

static lVal *wwlnfRenderDistance(lClosure *c, lVal *v){
	float rdist = 0.f;
	getLArgF(rdist);
	if(rdist > 1.f){ setRenderDistance(rdist); }
	return lValFloat(renderDistance);
}

static lVal *wwlnfSubData(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,v->vList.car));
		gfxUseSubData = t->vBool;
	}
	return lValBool(gfxUseSubData);
}

static lVal *wwlnfMouseSensitivity(lClosure *c, lVal *v){
	float msen = 0.f;
	getLArgF(msen);
	if(msen > 0.f){ optionMouseSensitivy = msen; }
	return lValFloat(optionMouseSensitivy);
}

static lVal *wwlnfThirdPerson(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,v->vList.car));
		optionThirdPerson = t->vBool;
	}
	return lValBool(optionThirdPerson);
}

static lVal *wwlnfFullscreen(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,v->vList.car));
		setFullscreen(t->vBool);
	}
	return lValBool(optionFullscreen);
}

static lVal *wwlnfWindowed(lClosure *c, lVal *v){
	int w = 800;
	int h = 600;
	int x = -1;
	int y = -1;

	getLArgI(w);
	getLArgI(h);
	getLArgI(x);
	getLArgI(y);

	setWindowed(w,h,x,y);
	return NULL;
}

static lVal *wwlnfDebugInfo(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,v->vList.car));
		optionDebugInfo = t->vBool;
	}
	return lValBool(optionDebugInfo);
}

static lVal *wwlnfConsMode(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,v->vList.car));
		if(t->vBool){
			player->flags |=  CHAR_CONS_MODE;
		}else{
			player->flags &= ~CHAR_CONS_MODE;
		}
	}
	return lValBool(player->flags & CHAR_CONS_MODE);
}

static lVal *wwlnfNoClip(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,v->vList.car));
		if(t->vBool != 0){
			player->flags |=  CHAR_NOCLIP;
		}else{
			player->flags &= ~CHAR_NOCLIP;
		}
	}
	return lValBool(player->flags & CHAR_NOCLIP);
}

static lVal *wwlnfWireFrame(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,v->vList.car));
		optionWireframe = t->vBool;
		initGL();
	}
	return lValBool(optionWireframe);
}

static lVal *wwlnfScreenshot(lClosure *c, lVal *v){
	(void)c;(void)v;
	queueScreenshot = true;
	return NULL;
}

static lVal *wwlnfSaveOptions(lClosure *c, lVal *v){
	(void)c;(void)v;
	saveOptions();
	return NULL;
}

static lVal *wwlnfServerAdd(lClosure *c, lVal *v){
	const char *address = "localhost";
	const char *name = "localhost";

	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfCat(c,lEval(c,v->vList.car));
		address = t->vString->buf;
		v = v->vList.cdr;
	}
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfCat(c,lEval(c,v->vList.car));
		name = t->vString->buf;
	}
	serverListAdd(address,name);

	return lValFloat(renderDistance);
}

static lVal *wwlnfPlayerPos(lClosure *c, lVal *v){
	(void)v;(void)c;
	return lValVec(player->pos);
}

static lVal *wwlnfPlayerRot(lClosure *c, lVal *v){
	(void)v;(void)c;
	return lValVec(player->rot);
}

static lVal *wwlnfPlayerVel(lClosure *c, lVal *v){
	(void)v;(void)c;
	return lValVec(player->vel);
}

static lVal *wwlnfFireHook(lClosure *c, lVal *v){
	(void)v;(void)c;
	characterFireHook(player);
	return NULL;
}

static lVal *wwlnfInvActiveSlot(lClosure *c, lVal *v){
	int ai = -1;
	getLArgI(ai);
	if(ai >= 0){
		player->activeItem = ai;
		player->flags &= ~(CHAR_AIMING | CHAR_THROW_AIM);
	}
	return lValInt(player->activeItem);
}


static lVal *wwlnfSendMessage(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	msgSendChatMessage(t->vString->data);
	return t;
}

static lVal *wwlnfConsolePrint(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCarOrV(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	widgetAddEntry(lispLog, t->vString->data);
	return t;
}

static lVal *wwlnfSfxPlay(lClosure *c, lVal *v){
	int sfxID    = -1;
	float volume = 1.0;
	vec pos      = player->pos;

	getLArgI(sfxID);
	getLArgF(volume);
	getLArgV(pos);

	if(sfxID < 0){return NULL;}
	sfxPlayPos(&sfxList[sfxID],volume,pos);
	return NULL;
}

static lVal *wwlnfResetWorstFrame(lClosure *c, lVal *v){
	(void)v;
	(void)c;
	worstFrame = 0;
	return NULL;
}

static lVal *wwlnfTextInputFocusPred(lClosure *c, lVal *v){
	(void)v;
	(void)c;
	return lValBool(textInputActive());
}

static lVal *wwlnfServerExecutable(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValString(clientGetServerExecutable());
}

static lVal *wwlnfTryToUse(lClosure *c, lVal *v){
	int ms     = 200;
	int amount = 1;
	item *itm  = &player->inventory[player->activeItem];

	getLArgI(ms);
	getLArgI(amount);

	bool ret = characterTryToUse(player,itm,ms,amount);
	return lValBool(ret);
}

static lVal *wwlnfStartAnim(lClosure *c, lVal *v){
	int anim = 0;
	int ms = 200;

	getLArgI(anim);
	getLArgI(ms);

	characterStartAnimation(player,anim,ms);
	return NULL;
}

static lVal *wwlnfStopAnim(lClosure *c, lVal *v){
	(void)c;(void)v;
	characterStopAnimation(player);
	return NULL;
}

static lVal *wwlnfGrenadeNew(lClosure *c, lVal *v){
	vec pos          = player->pos;
	vec rot          = player->rot;
	float pwr        = 4.f;
	int cluster      = 0;
	float clusterPwr = 0.f;

	getLArgV(pos);
	getLArgV(rot);
	getLArgF(pwr);
	getLArgI(cluster);
	getLArgF(clusterPwr);

	grenadeNew(pos,rot,pwr,cluster,clusterPwr);
	return NULL;
}

static lVal *wwlnfTryToThrow(lClosure *c, lVal *v){
	(void)c;(void)v;
	item *itm = &player->inventory[player->activeItem];
	return lValBool(throwableTryAim(itm,player));
}

static lVal *wwlnfItemReload(lClosure *c, lVal *v){
	int ms     = 200;
	item *itm = &player->inventory[player->activeItem];

	getLArgI(ms);

	characterItemReload(player,itm,ms);
	return NULL;
}

static lVal *wwlnfToggleAim(lClosure *c, lVal *v){
	float zoom = 2.f;

	getLArgF(zoom);

	characterToggleAim(player,zoom);
	return NULL;
}

static lVal *wwlnfInaccuracy(lClosure *c, lVal *v){
	float inacc = -1024.f;
	getLArgF(inacc);
	if(inacc > -1024.f){characterSetInaccuracy(player,inacc);}
	return lValFloat(player->inaccuracy);
}

static lVal *wwlnfRecoil(lClosure *c, lVal *v){
	float recoil = 1.f;
	getLArgF(recoil);
	characterAddRecoil(player,recoil);
	return NULL;
}

static lVal *wwlnfPlayerHP(lClosure *c, lVal *v){
	int hp = -1024;
	getLArgI(hp);
	if(hp > -1024){ player->hp = MIN(player->maxhp,hp); }
	return lValInt(player->hp);
}

static lVal *wwlnfPlayerMaxHP(lClosure *c, lVal *v){
	int maxhp = -1024;
	getLArgI(maxhp);
	if(maxhp > -1024){ player->maxhp = MAX(1,maxhp); }
	return lValInt(player->maxhp);
}

static lVal *wwlnfRResult(lClosure *c, lVal *v){
	int id     = -1;
	int result = 0;
	int amount = 0;

	getLArgI(id);
	getLArgI(result);
	getLArgI(amount);

	if((id < 0) || (result <= 0) || (amount <= 0)){return NULL;}
	recipeCount = MAX(id+1,(int)recipeCount);
	recipes[id].result.ID = result;
	recipes[id].result.amount = amount;
	return NULL;
}

static lVal *wwlnfRIngred(lClosure *c, lVal *v){
	int id = -1;
	int ii = -1;
	int ingred = 0;
	int amount = 0;

	getLArgI(id);
	getLArgI(ii);
	getLArgI(ingred);
	getLArgI(amount);

	if((id < 0) || (ii < 0) || (ii >= 4) || (ingred <= 0) || (amount <= 0)){return NULL;}
	recipeCount = MAX(id+1,(int)recipeCount);
	recipes[id].ingredient[ii].ID = ingred;
	recipes[id].ingredient[ii].amount = amount;
	return NULL;
}

static lVal *wwlnfAimingPred(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValBool(characterIsAiming(player));
}

static lVal *wwlnfThrowingPred(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValBool(characterIsThrowAiming(player));
}

static lVal *wwlnfThrowItem(lClosure *c, lVal *v){
	int flags   = 0;
	float force = 0.1f;
	int damage  = 1;

	getLArgI(flags);
	getLArgF(force);
	getLArgI(damage);

	item *itm = &player->inventory[player->activeItem];
	bool ret  = throwableTry(itm,player,force,damage,flags);
	return lValBool(ret);
}

static lVal *wwlnfTryToShoot(lClosure *c, lVal *v){
	int cooldown    = 200;
	int bulletcount = 1;

	getLArgI(cooldown);
	getLArgI(bulletcount);

	item *itm = &player->inventory[player->activeItem];
	bool ret  = characterTryToShoot(player,itm,cooldown,bulletcount);
	return lValBool(ret);
}

static lVal *wwlnfBeamblast(lClosure *c, lVal *v){
	float beamSize = 1.f;
	float damage   = 8.f;
	int hitsLeft   = 3;

	getLArgF(beamSize);
	getLArgF(damage);
	getLArgI(hitsLeft);

	beamblast(player,beamSize,damage,hitsLeft);
	return NULL;
}

static lVal *wwlnfProjectile(lClosure *c, lVal *v){
	int type = 0;
	int num  = 1;

	getLArgI(type);
	getLArgI(num);

	projectileNewC(player,type,num);
	return NULL;
}

static lVal *wwlnfFireNew(lClosure *c, lVal *v){
	vec pos = vecNOne();
	int str = 8;

	getLArgV(pos);
	getLArgI(str);

	if(vecInWorld(pos)){fireNew(pos.x,pos.y,pos.z,str);}
	return NULL;
}

static lVal *wwlnfRaycast(lClosure *c, lVal *v){
	bool before = false;
	getLArgB(before);
	return lValVec(vecNewI(characterLOSBlock(player,before)));
}

static lVal *wwlnfPlayerInventory(lClosure *c, lVal *v){
	int slot   = -1;
	int itemID = -1;
	int amount = -1;

	getLArgI(slot);
	getLArgI(itemID);
	getLArgI(amount);
	if(slot < 0){return NULL;}
	if((itemID > 0) && (amount > 0)){
		player->inventory[slot] = itemNew(itemID,amount);
	}
	return lValInt(player->inventory[slot].ID);
}

void addClientNFuncs(lClosure *c){
	lAddNativeFunc(c,"s",              "(...body)",        "Evaluates ...body on the serverside and returns the last result",wwlnfSEval);
	lAddNativeFunc(c,"text-focus?",    "()",               "Returns if a text input field is currently focused",             wwlnfTextInputFocusPred);
	lAddNativeFunc(c,"player-pos",     "()",               "Returns players position",                                       wwlnfPlayerPos);
	lAddNativeFunc(c,"player-rot",     "()",               "Returns players rotation",                                       wwlnfPlayerRot);
	lAddNativeFunc(c,"player-vel",     "()",               "Returns players velocity",                                       wwlnfPlayerVel);
	lAddNativeFunc(c,"player-name!",   "(s)",              "Sets players name to s",                                         wwlnfPlayerName);
	lAddNativeFunc(c,"player-hp",      "(&hp)",            "Sets the players health to &HP, returns the current value.",     wwlnfPlayerHP);
	lAddNativeFunc(c,"player-maxhp",   "(&mhp)",           "Sets the players max health to &MHP, returns the current value.",wwlnfPlayerMaxHP);
	lAddNativeFunc(c,"sound-vol!",     "(f)",              "Sets sound volume to float f",                                   wwlnfSoundVolume);
	lAddNativeFunc(c,"view-dist!",     "(f)",              "Sets render distance to f blocks",                               wwlnfRenderDistance);
	lAddNativeFunc(c,"use-sub-data!",  "(b)",              "Sets useSubdata boolean to b",                                   wwlnfSubData);
	lAddNativeFunc(c,"mouse-sens!",    "(f)",              "Sets the mouse sensitivity to f",                                wwlnfMouseSensitivity);
	lAddNativeFunc(c,"server-add!",    "(name ip)",        "Adds name ip to server list",                                    wwlnfServerAdd);
	lAddNativeFunc(c,"third-person!",  "(b)",              "Sets third person view to b",                                    wwlnfThirdPerson);
	lAddNativeFunc(c,"fullscreen",     "(b)",              "Sets fullscreen to b",                                           wwlnfFullscreen);
	lAddNativeFunc(c,"windowed",       "(&w &h &x &y)",    "Switches to windowed mode of size &w/&h at position &x/&y",      wwlnfWindowed);
	lAddNativeFunc(c,"save-options",   "()",               "Save options to disk",                                           wwlnfSaveOptions);
	lAddNativeFunc(c,"reset-worst-f",  "()",               "Resets the worst frame counter",                                 wwlnfResetWorstFrame);
	lAddNativeFunc(c,"debug-info!",    "(b)",              "Sets debug info view to b",                                      wwlnfDebugInfo);
	lAddNativeFunc(c,"cons-mode!",     "(b)",              "Sets cons-mode to b if passed, always returns the current state",wwlnfConsMode);
	lAddNativeFunc(c,"no-clip!",       "(b)",              "Sets no clip to b if passed, always returns the current state",  wwlnfNoClip);
	lAddNativeFunc(c,"wire-frame!",    "(b)",              "Sets wireframe mode to b, always returns the current state",     wwlnfWireFrame);
	lAddNativeFunc(c,"send-message",   "(s)",              "Sends string s as a chat message",                               wwlnfSendMessage);
	lAddNativeFunc(c,"console-print",  "(s)",              "Prints string s to the REPL",                                    wwlnfConsolePrint);
	lAddNativeFunc(c,"sfx-play",       "(s &vol &pos)",    "Plays SFX S with volume &VOL=1.0 as if emitting from &POS.",     wwlnfSfxPlay);
	lAddNativeFunc(c,"screenshot",     "()",               "Takes a screeshot",                                              wwlnfScreenshot);
	lAddNativeFunc(c,"fire-hook",      "()",               "Fires the players Grappling hook, or retracts it if fired",      wwlnfFireHook);
	lAddNativeFunc(c,"inv-active-slot","(&i)",             "Get/Set the players active slot to I",                           wwlnfInvActiveSlot);
	lAddNativeFunc(c,"player-inv",     "(i &id &amount)",  "Get/Set the players invetory slot I to &ID and &AMOUNT",         wwlnfPlayerInventory);
	lAddNativeFunc(c,"server-path",    "()",               "Returns the path to the server executable, if found.",           wwlnfServerExecutable);
	lAddNativeFunc(c,"try-to-use",     "(&ms &amount)",    "Try to use &AMOUNT=1 and wait for &MS=200.",                     wwlnfTryToUse);
	lAddNativeFunc(c,"start-anim",     "(id ms)",          "Starts animation &ID=0 for &MS=200",                             wwlnfStartAnim);
	lAddNativeFunc(c,"stop-anim" ,     "()",               "Stops any animation that is currently playing",                  wwlnfStopAnim);
	lAddNativeFunc(c,"grenade-new",    "(pos rot pwr cluster clusterpwr)", "Creates a new grenade a POS moving into ROT creating an explosion of size PWR and then splitting into CLUSTER grenades with a power of CLUSTERPWR.", wwlnfGrenadeNew);
	lAddNativeFunc(c,"try-to-throw",   "()",               "Try to switch into a throwing mode for the currently held item", wwlnfTryToThrow);
	lAddNativeFunc(c,"item-reload",    "(&ms)",            "Reloads the currently held item in &MS=200.",                    wwlnfItemReload);
	lAddNativeFunc(c,"toggle-aim",     "(&zoom)",          "Toggles aiming with a &ZOOM=4 factor.",                          wwlnfToggleAim);
	lAddNativeFunc(c,"inaccuracy",     "(&acc)",           "Set the player inaccuracy to &ACC if set, return inaccuracy",    wwlnfInaccuracy);
	lAddNativeFunc(c,"recoil",         "(&rec)",           "Adds recoil to the player of &REC=1.0",                          wwlnfRecoil);
	lAddNativeFunc(c,"r-result",       "(id result amt)",  "Set the result of recipe ID to AMT times RESULT.",               wwlnfRResult);
	lAddNativeFunc(c,"r-ingred",       "(id i ingred amt)","Set the ingredient I of recipe ID to AMT times INGRED.",         wwlnfRIngred);
	lAddNativeFunc(c,"aiming?",        "()",               "Predicate that evaluates to #t when the player is aiming.",      wwlnfAimingPred);
	lAddNativeFunc(c,"throwing?",      "()",               "Predicate that evaluates to #t when the player is throw-aiming.",wwlnfThrowingPred);
	lAddNativeFunc(c,"throw-item",     "(flags &force &amount)", "Throw the currently held item with FLAGS, &FORCE=0.1 and &AMOUNT=1",wwlnfThrowItem);
	lAddNativeFunc(c,"try-to-shoot",   "(cd ammo)",        "Try to shoot and cooldown for CD and use AMMO bullets"          ,wwlnfTryToShoot);
	lAddNativeFunc(c,"beamblast",      "(size damage hits-left)", "Calls cFunc beamblast",                                   wwlnfBeamblast);
	lAddNativeFunc(c,"projectile",     "(&type &num)",      "Fire &NUM=1 projectiles of &TYPE=0",                            wwlnfProjectile);
	lAddNativeFunc(c,"fire-new",       "(pos &strength)",   "Create/Grow a fire at POS with &STRENGTH=8",                    wwlnfFireNew);
	lAddNativeFunc(c,"player-raycast", "(beforeBlock)",     "Return the position of the first intersection, or before if BEFOREBLOCK is #t", wwlnfRaycast);
}

void lispInit(){
	lInit();
	clRoot = lispCommonRoot();
	addClientNFuncs(clRoot);
	lEval(clRoot,lWrap(lRead((const char *)src_tmp_client_nuj_data)));
	lClosureGC();
}

void lispFree(){
	lClosureFree(clRoot);
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

void lispRecvSExpr(const packet *p){
	u8 id = p->v.u8[0];
	const char *str = (const char *)&p->v.u8[1];
	lispPanelShowReply(lispSEvalSym(id),str);
}

void lispEvents(){
	PROFILE_START();

	static u64 lastTicks = 0;
	u64 cticks = getTicks();
	if((lastTicks + 500) > cticks){
		if(lastTicks > cticks){lastTicks = cticks;}
		return;
	}
	lastTicks = cticks;

	lispEval("(begin (yield-run))");
	lClosureGC();

	PROFILE_STOP();
}
