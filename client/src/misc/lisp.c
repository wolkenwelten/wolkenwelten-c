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
#include "../gui/widget.h"
#include "../misc/options.h"
#include "../menu/inventory.h"
#include "../network/chat.h"
#include "../network/client.h"
#include "../sdl/sdl.h"
#include "../sdl/sfx.h"
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

extern        size_t src_tmp_client_nuj_len;
extern unsigned char src_tmp_client_nuj_data[];

void lispInputHandler(lSymbol *input, int key, int action){
	lVal *expr;
	lVal *lInputS = lValSymS(input);
	lVal *lAction = lValInt(action);
	lVal *lCode   = lValInt(key);
	expr = lCons(lCons(lInputS,lCons(lCode,NULL)),lCons(lCode,lCons(lAction,NULL)));
	lEval(clRoot,expr);
}

void lispInputTick(){
	static lSymbol *keyInput = NULL;
	if(keyInput == NULL){keyInput = lSymS("input-tick");}
	lEval(clRoot,lCons(lCons(lValSymS(keyInput),NULL),NULL));
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
	lSWriteVal(lWrap(v),buf,&buf[sizeof(buf)-1],0,false);
	msgLispSExpr(-1,buf);
	return NULL;
}

static lVal *wwlnfPlayerName(lClosure *c, lVal *v){
	const char *npName = NULL;

	v = getLArgS(c,v,&npName);

	if(npName != NULL){
		snprintf(playerName,sizeof(playerName),"%s",npName);
	}
	return lValString(playerName);
}

static lVal *wwlnfSoundVolume(lClosure *c, lVal *v){
	float nvol = -1.f;
	v = getLArgF(c,v,&nvol);
	if(nvol > -0.1f){optionSoundVolume = nvol;}
	return lValFloat(optionSoundVolume);
}

static lVal *wwlnfRenderDistance(lClosure *c, lVal *v){
	float rdist = 0.f;
	v = getLArgF(c,v,&rdist);
	if(rdist > 1.f){ setRenderDistance(rdist); }
	return lValFloat(renderDistance);
}

static lVal *wwlnfSubData(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,lCar(v)));
		gfxUseSubData = t->vBool;
	}
	return lValBool(gfxUseSubData);
}

static lVal *wwlnfMouseSensitivity(lClosure *c, lVal *v){
	float msen = 0.f;
	v = getLArgF(c,v,&msen);
	if(msen > 0.f){ optionMouseSensitivy = msen; }
	return lValFloat(optionMouseSensitivy);
}

static lVal *wwlnfThirdPerson(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,lCar(v)));
		optionThirdPerson = t->vBool;
	}
	return lValBool(optionThirdPerson);
}

static lVal *wwlnfFullscreen(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,lCar(v)));
		setFullscreen(t->vBool);
	}
	return lValBool(optionFullscreen);
}

static lVal *wwlnfWindowed(lClosure *c, lVal *v){
	int w = 800;
	int h = 600;
	int x = -1;
	int y = -1;

	v = getLArgI(c,v,&w);
	v = getLArgI(c,v,&h);
	v = getLArgI(c,v,&x);
	v = getLArgI(c,v,&y);

	setWindowed(w,h,x,y);
	return NULL;
}

static lVal *wwlnfDebugInfo(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,lCar(v)));
		optionDebugInfo = t->vBool;
	}
	return lValBool(optionDebugInfo);
}

static lVal *wwlnfConsMode(lClosure *c, lVal *v){
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfBool(c,lEval(c,lCar(v)));
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
		lVal *t = lnfBool(c,lEval(c,lCar(v)));
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
		lVal *t = lnfBool(c,lEval(c,lCar(v)));
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
		lVal *t = lnfCat(c,lEval(c,lCar(v)));
		address = lStrBuf(t);
		v = lCdr(v);
	}
	if((v != NULL) && (v->type == ltPair)){
		lVal *t = lnfCat(c,lEval(c,lCar(v)));
		name = lStrBuf(t);
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

static lVal *wwlnfPlayerActiveSlotGet(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(player->activeItem);
}

static lVal *wwlnfPlayerActiveSlotSet(lClosure *c, lVal *v){
	int ai = -1;
	v = getLArgI(c,v,&ai);
	if(ai >= 0){
		player->activeItem = ai;
		player->flags &= ~(CHAR_AIMING | CHAR_THROW_AIM);
	}
	return NULL;
}

static lVal *wwlnfSendMessage(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltString)){return NULL;}
	msgSendChatMessage(lStrData(t));
	return t;
}

static lVal *wwlnfConsolePrint(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltString) || lStrNull(t)){return NULL;}
	widgetAddEntry(lispLog, lStrData(t));
	return t;
}

static lVal *wwlnfSfxPlay(lClosure *c, lVal *v){
	int sfxID    = -1;
	float volume = 1.0;
	vec pos      = player->pos;

	v = getLArgI(c,v,&sfxID);
	v = getLArgF(c,v,&volume);
	v = getLArgV(c,v,&pos);

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

	v = getLArgI(c,v,&ms);
	v = getLArgI(c,v,&amount);

	bool ret = characterTryToUse(player,itm,ms,amount);
	return lValBool(ret);
}

static lVal *wwlnfStartAnim(lClosure *c, lVal *v){
	int anim = 0;
	int ms = 200;

	v = getLArgI(c,v,&anim);
	v = getLArgI(c,v,&ms);

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

	v = getLArgV(c,v,&pos);
	v = getLArgV(c,v,&rot);
	v = getLArgF(c,v,&pwr);
	v = getLArgI(c,v,&cluster);
	v = getLArgF(c,v,&clusterPwr);

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

	v = getLArgI(c,v,&ms);

	characterItemReload(player,itm,ms);
	return NULL;
}

static lVal *wwlnfToggleAim(lClosure *c, lVal *v){
	float zoom = 2.f;

	v = getLArgF(c,v,&zoom);

	characterToggleAim(player,zoom);
	return NULL;
}

static lVal *wwlnfInaccuracy(lClosure *c, lVal *v){
	float inacc = -1024.f;
	v = getLArgF(c,v,&inacc);
	if(inacc > -1024.f){characterSetInaccuracy(player,inacc);}
	return lValFloat(player->inaccuracy);
}

static lVal *wwlnfRecoil(lClosure *c, lVal *v){
	float recoil = 1.f;
	v = getLArgF(c,v,&recoil);
	characterAddRecoil(player,recoil);
	return NULL;
}

static lVal *wwlnfPlayerHP(lClosure *c, lVal *v){
	int hp = -1024;
	v = getLArgI(c,v,&hp);
	if(hp > -1024){ player->hp = MIN(player->maxhp,hp); }
	return lValInt(player->hp);
}

static lVal *wwlnfPlayerMaxHP(lClosure *c, lVal *v){
	int maxhp = -1024;
	v = getLArgI(c,v,&maxhp);
	if(maxhp > -1024){ player->maxhp = MAX(1,maxhp); }
	return lValInt(player->maxhp);
}

static lVal *wwlnfRResult(lClosure *c, lVal *v){
	int id     = -1;
	int result = 0;
	int amount = 0;

	v = getLArgI(c,v,&id);
	v = getLArgI(c,v,&result);
	v = getLArgI(c,v,&amount);

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

	v = getLArgI(c,v,&id);
	v = getLArgI(c,v,&ii);
	v = getLArgI(c,v,&ingred);
	v = getLArgI(c,v,&amount);

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

	v = getLArgI(c,v,&flags);
	v = getLArgF(c,v,&force);
	v = getLArgI(c,v,&damage);

	item *itm = &player->inventory[player->activeItem];
	bool ret  = throwableTry(itm,player,force,damage,flags);
	return lValBool(ret);
}

static lVal *wwlnfTryToShoot(lClosure *c, lVal *v){
	int cooldown    = 200;
	int bulletcount = 1;

	v = getLArgI(c,v,&cooldown);
	v = getLArgI(c,v,&bulletcount);

	item *itm = &player->inventory[player->activeItem];
	bool ret  = characterTryToShoot(player,itm,cooldown,bulletcount);
	return lValBool(ret);
}

static lVal *wwlnfBeamblast(lClosure *c, lVal *v){
	float beamSize = 1.f;
	float damage   = 8.f;
	int hitsLeft   = 3;

	v = getLArgF(c,v,&beamSize);
	v = getLArgF(c,v,&damage);
	v = getLArgI(c,v,&hitsLeft);

	beamblast(player,beamSize,damage,hitsLeft);
	return NULL;
}

static lVal *wwlnfProjectile(lClosure *c, lVal *v){
	int type = 0;
	int num  = 1;

	v = getLArgI(c,v,&type);
	v = getLArgI(c,v,&num);

	projectileNewC(player,type,num);
	return NULL;
}

static lVal *wwlnfFireNew(lClosure *c, lVal *v){
	vec pos = vecNOne();
	int str = 8;

	v = getLArgV(c,v,&pos);
	v = getLArgI(c,v,&str);

	if(vecInWorld(pos)){fireNew(pos.x,pos.y,pos.z,str);}
	return NULL;
}

static lVal *wwlnfRaycast(lClosure *c, lVal *v){
	bool before = false;

	v = getLArgB(c,v,&before);

	return lValVec(vecNewI(characterLOSBlock(player,before)));
}

static lVal *wwlnfWindowWidth(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(screenWidth);
}

static lVal *wwlnfWindowHeight(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(screenHeight);
}

static lVal *wwlnfMouseHidden(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValBool(mouseHidden);
}

static lVal *wwlnfPlayerDoPrimary(lClosure *c, lVal *v){
	(void)c;(void)v;
	characterDoPrimary(player);
	return NULL;
}

static lVal *wwlnfPlayerStopMining(lClosure *c, lVal *v){
	(void)c;(void)v;
	characterStopMining(player);
	return NULL;
}

static lVal *wwlnfPlayerInventoryGet(lClosure *c, lVal *v){
	int slot   = -1;

	v = getLArgI(c,v,&slot);

	if(slot < 0)                               {return NULL;}
	if(slot >= (int)countof(player->inventory)){return NULL;}
	const item *itm = &player->inventory[slot];
	return lCons(lValInt(itm->ID),lValInt(itm->amount));
}

static lVal *wwlnfPlayerInventorySet(lClosure *c, lVal *v){
	int slot   = -1;
	int itemID = -1;
	int amount = -1;

	v = getLArgI(c,v,&slot);
	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&amount);

	if(slot < 0)                               {return NULL;}
	if(slot >= (int)countof(player->inventory)){return NULL;}
	if((itemID > 0) && (amount > 0)){
		player->inventory[slot] = itemNew(itemID,amount);
	}
	return NULL;
}

static lVal *wwlnfPlayerPlaceBlock(lClosure *c, lVal *v){
	int slot   = -1;

	v = getLArgI(c,v,&slot);

	if(slot < 0)                               {return NULL;}
	if(slot >= (int)countof(player->inventory)){return NULL;}
	item *itm = &player->inventory[slot];
	characterPlaceBlock(player,itm);

	return NULL;
}

static lVal *wwlnfPlayerZoomGet(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValFloat(player->zoomFactor);
}

static lVal *wwlnfPlayerZoomSet(lClosure *c, lVal *v){
	float zoom   = -1.0;

	v = getLArgF(c,v,&zoom);

	zoom = MINMAX(zoom,1.f,8.f);
	if(zoom > 1.01f){
		player->goalZoomFactor = zoom;
		player->flags |=   CHAR_AIMING | CHAR_THROW_AIM;
	}else{
		player->goalZoomFactor = 1.f;
		player->flags &= ~(CHAR_AIMING | CHAR_THROW_AIM);
	}

	return NULL;
}


static lVal *wwlnfDropItem(lClosure *c, lVal *v){
	int slot   = -1;

	v = getLArgI(c,v,&slot);

	if(slot < 0)                               {return NULL;}
	if(slot >= (int)countof(player->inventory)){return NULL;}
	characterDropSingleItem(player,slot);

	return NULL;
}

static lVal *wwlnfPlayerJump(lClosure *c, lVal *v){
	float vel = 0;

	v = getLArgF(c,v,&vel);
	player->controls.y = vel;

	return NULL;
}

static lVal *wwlnfPlayerSneak(lClosure *c, lVal *v){
	bool active = false;

	v = getLArgB(c,v,&active);
	if(active){
		player->flags |=  CHAR_SNEAK;
	}else{
		player->flags &= ~CHAR_SNEAK;
	}

	return NULL;
}

static lVal *wwlnfPlayerWalk(lClosure *c, lVal *v){
	float vel = 0;

	v = getLArgF(c,v,&vel);
	player->controls.z = vel;

	return NULL;
}

static lVal *wwlnfPlayerStrafe(lClosure *c, lVal *v){
	float vel = 0;

	v = getLArgF(c,v,&vel);
	player->controls.x = vel;

	return NULL;
}

static lVal *wwlnfToggleInventory(lClosure *c, lVal *v){
	(void)c;(void)v;
	toggleInventory();
	return NULL;
}

static lVal *wwlnfPlayerBoost(lClosure *c, lVal *v){
	bool active = false;

	v = getLArgB(c,v,&active);
	if(active){
		player->flags |=  CHAR_BOOSTING;
	}else{
		player->flags &= ~CHAR_BOOSTING;
	}

	return NULL;
}

static lVal *wwlnfGuiFocusOnGame(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValBool(!gameControlsInactive());
}

static void lispAddClientNFuncs(lClosure *c){
	lAddNativeFunc(c,"s",              "(...body)",         "Evaluates ...body on the serverside and returns the last result",wwlnfSEval);
	lAddNativeFunc(c,"text-focus?",    "()",                "Returns if a text input field is currently focused",             wwlnfTextInputFocusPred);
	lAddNativeFunc(c,"player-pos",     "()",                "Returns players position",                                       wwlnfPlayerPos);
	lAddNativeFunc(c,"player-rot",     "()",                "Returns players rotation",                                       wwlnfPlayerRot);
	lAddNativeFunc(c,"player-vel",     "()",                "Returns players velocity",                                       wwlnfPlayerVel);
	lAddNativeFunc(c,"player-name!",   "(s)",               "Sets players name to s",                                         wwlnfPlayerName);
	lAddNativeFunc(c,"player-hp",      "(&hp)",             "Sets the players health to &HP, returns the current value.",     wwlnfPlayerHP);
	lAddNativeFunc(c,"player-maxhp",   "(&mhp)",            "Sets the players max health to &MHP, returns the current value.",wwlnfPlayerMaxHP);
	lAddNativeFunc(c,"player-jump!",   "(velocity)",        "Jump with VELOCITY!",                                            wwlnfPlayerJump);
	lAddNativeFunc(c,"player-sneak!",  "()",                "Sneak for a while",                                              wwlnfPlayerSneak);
	lAddNativeFunc(c,"player-walk!",   "(velocity)",        "Walk forward with VELOCITY",                                     wwlnfPlayerWalk);
	lAddNativeFunc(c,"player-strafe!", "(velocity)",        "Strafe sideways with VEOLOCITY",                                 wwlnfPlayerStrafe);
	lAddNativeFunc(c,"player-boost!" , "(active)",          "Set Boost to ACTIVE",                                            wwlnfPlayerBoost);
	lAddNativeFunc(c,"player-raycast", "(beforeBlock)",     "Return the position of the first intersection, or before if BEFOREBLOCK is #t", wwlnfRaycast);
	lAddNativeFunc(c,"player-do-primary!", "()",            "Let the player Mine/Hit",                                        wwlnfPlayerDoPrimary);
	lAddNativeFunc(c,"player-stop-mining!","()",            "Stop mining",                                                    wwlnfPlayerStopMining);
	lAddNativeFunc(c,"player-place-block!","(slot)",        "Places the block in SLOT in front the player",                   wwlnfPlayerPlaceBlock);
	lAddNativeFunc(c,"player-zoom",    "()",                "Return the current zoom factor",                                 wwlnfPlayerZoomGet);
	lAddNativeFunc(c,"player-zoom!",   "(zoom)",            "Set the current ZOOM factor",                                    wwlnfPlayerZoomSet);
	lAddNativeFunc(c,"sound-vol!",     "(f)",               "Sets sound volume to float f",                                   wwlnfSoundVolume);
	lAddNativeFunc(c,"view-dist!",     "(f)",               "Sets render distance to f blocks",                               wwlnfRenderDistance);
	lAddNativeFunc(c,"use-sub-data!",  "(b)",               "Sets useSubdata boolean to b",                                   wwlnfSubData);
	lAddNativeFunc(c,"mouse-sens!",    "(f)",               "Sets the mouse sensitivity to f",                                wwlnfMouseSensitivity);
	lAddNativeFunc(c,"mouse-hidden",   "()",                "Return if the mouse cursor is currently hidden",                 wwlnfMouseHidden);
	lAddNativeFunc(c,"server-add!",    "(name ip)",         "Adds name ip to server list",                                    wwlnfServerAdd);
	lAddNativeFunc(c,"third-person!",  "(b)",               "Sets third person view to b",                                    wwlnfThirdPerson);
	lAddNativeFunc(c,"fullscreen",     "(b)",               "Sets fullscreen to b",                                           wwlnfFullscreen);
	lAddNativeFunc(c,"windowed",       "(&w &h &x &y)",     "Switches to windowed mode of size &w/&h at position &x/&y",      wwlnfWindowed);
	lAddNativeFunc(c,"window-width",   "()",                "Returns the width of the widow in pixels",                       wwlnfWindowWidth);
	lAddNativeFunc(c,"window-height",  "()",                "Returns the height of the widow in pixels",                      wwlnfWindowHeight);
	lAddNativeFunc(c,"save-options",   "()",                "Save options to disk",                                           wwlnfSaveOptions);
	lAddNativeFunc(c,"reset-worst-f",  "()",                "Resets the worst frame counter",                                 wwlnfResetWorstFrame);
	lAddNativeFunc(c,"debug-info!",    "(b)",               "Sets debug info view to b",                                      wwlnfDebugInfo);
	lAddNativeFunc(c,"cons-mode!",     "(b)",               "Sets cons-mode to b if passed, always returns the current state",wwlnfConsMode);
	lAddNativeFunc(c,"no-clip!",       "(b)",               "Sets no clip to b if passed, always returns the current state",  wwlnfNoClip);
	lAddNativeFunc(c,"wire-frame!",    "(b)",               "Sets wireframe mode to b, always returns the current state",     wwlnfWireFrame);
	lAddNativeFunc(c,"send-message",   "(s)",               "Sends string s as a chat message",                               wwlnfSendMessage);
	lAddNativeFunc(c,"console-print",  "(s)",               "Prints string s to the REPL",                                    wwlnfConsolePrint);
	lAddNativeFunc(c,"sfx-play",       "(s &vol &pos)",     "Plays SFX S with volume &VOL=1.0 as if emitting from &POS.",     wwlnfSfxPlay);
	lAddNativeFunc(c,"screenshot",     "()",                "Takes a screeshot",                                              wwlnfScreenshot);
	lAddNativeFunc(c,"fire-hook",      "()",                "Fires the players Grappling hook, or retracts it if fired",      wwlnfFireHook);
	lAddNativeFunc(c,"player-active-slot", "()",            "Get the players active slot",                                    wwlnfPlayerActiveSlotGet);
	lAddNativeFunc(c,"player-active-slot!","(slot)",        "Set the players active SLOT",                                    wwlnfPlayerActiveSlotSet);
	lAddNativeFunc(c,"player-inventory",   "(slot)",           "Get the players inventory SLOT",                              wwlnfPlayerInventoryGet);
	lAddNativeFunc(c,"player-inventory!",  "(slot id &amount)","Set the players inventory SLOT to ID and AMOUNT",             wwlnfPlayerInventorySet);
	lAddNativeFunc(c,"server-path",    "()",                "Returns the path to the server executable, if found.",           wwlnfServerExecutable);
	lAddNativeFunc(c,"try-to-use",     "(&ms &amount)",     "Try to use &AMOUNT=1 and wait for &MS=200.",                     wwlnfTryToUse);
	lAddNativeFunc(c,"start-anim",     "(id ms)",           "Starts animation &ID=0 for &MS=200",                             wwlnfStartAnim);
	lAddNativeFunc(c,"stop-anim" ,     "()",                "Stops any animation that is currently playing",                  wwlnfStopAnim);
	lAddNativeFunc(c,"grenade-new",    "(pos rot pwr cluster clusterpwr)", "Creates a new grenade a POS moving into ROT creating an explosion of size PWR and then splitting into CLUSTER grenades with a power of CLUSTERPWR.", wwlnfGrenadeNew);
	lAddNativeFunc(c,"try-to-throw",   "()",                "Try to switch into a throwing mode for the currently held item", wwlnfTryToThrow);
	lAddNativeFunc(c,"item-reload",    "(&ms)",             "Reloads the currently held item in &MS=200.",                    wwlnfItemReload);
	lAddNativeFunc(c,"toggle-aim",     "(&zoom)",           "Toggles aiming with a &ZOOM=4 factor.",                          wwlnfToggleAim);
	lAddNativeFunc(c,"inaccuracy",     "(&acc)",            "Set the player inaccuracy to &ACC if set, return inaccuracy",    wwlnfInaccuracy);
	lAddNativeFunc(c,"recoil",         "(&rec)",            "Adds recoil to the player of &REC=1.0",                          wwlnfRecoil);
	lAddNativeFunc(c,"r-result",       "(id result amt)",   "Set the result of recipe ID to AMT times RESULT.",               wwlnfRResult);
	lAddNativeFunc(c,"r-ingred",       "(id i ingred amt)", "Set the ingredient I of recipe ID to AMT times INGRED.",         wwlnfRIngred);
	lAddNativeFunc(c,"aiming?",        "()",                "Predicate that evaluates to #t when the player is aiming.",      wwlnfAimingPred);
	lAddNativeFunc(c,"throwing?",      "()",                "Predicate that evaluates to #t when the player is throw-aiming.",wwlnfThrowingPred);
	lAddNativeFunc(c,"throw-item",     "(flags &force &amount)", "Throw the currently held item with FLAGS, &FORCE=0.1 and &AMOUNT=1",wwlnfThrowItem);
	lAddNativeFunc(c,"player-drop-item!","(slot)",          "Throw a single item from SLOT in front of the player" ,wwlnfDropItem);
	lAddNativeFunc(c,"try-to-shoot",   "(cd ammo)",         "Try to shoot and cooldown for CD and use AMMO bullets"          ,wwlnfTryToShoot);
	lAddNativeFunc(c,"beamblast",      "(size damage hits-left)", "Calls cFunc beamblast",                                   wwlnfBeamblast);
	lAddNativeFunc(c,"projectile",     "(&type &num)",      "Fire &NUM=1 projectiles of &TYPE=0",                            wwlnfProjectile);
	lAddNativeFunc(c,"fire-new",       "(pos &strength)",   "Create/Grow a fire at POS with &STRENGTH=8",                    wwlnfFireNew);
	lAddNativeFunc(c,"toggle-inventory!","()",              "Toggle the inveotory",                                          wwlnfToggleInventory);
	lAddNativeFunc(c,"widget-focus-on-game?","()",          "Return #t if the game is focused and not some menu",            wwlnfGuiFocusOnGame);
}

void lispInit(){
	lInit();
	clRoot = lispCommonRoot();
	lispAddClientNFuncs(clRoot);
	widgetAddLispFunctions(clRoot);
	lEval(clRoot,lWrap(lRead((const char *)src_tmp_client_nuj_data)));
	lClosureGC();
}

void lispFree(){
	lClosureFree(clRoot - lClosureList);
}

const char *lispEval(const char *str){
	static char reply[4096];
	memset(reply,0,sizeof(reply));
	lVal *v = lnfBegin(clRoot,lRead(str));
	lSWriteVal(v,reply,&reply[sizeof(reply)-1],0,false);

	int soff,slen,len = strnlen(reply,sizeof(reply)-1);
	for(soff = 0;    isspace((u8)reply[soff]) || (reply[soff] == '"');soff++){}
	for(slen = len-1;isspace((u8)reply[slen]) || (reply[slen] == '"');slen--){reply[slen] = 0;}

	return reply+soff;
}

lVal *lispEvalL(lVal *expr){
	return lEval(clRoot,expr);
}

void lispRecvSExpr(const packet *p){
	lispPanelShowReply((const char *)&p->v.u8);
}

void lispEvents(){
	PROFILE_START();

	static u64 lastTicks = 0;
	lClosureGC();
	u64 cticks = getTicks();
	if((lastTicks + 20) > cticks){
		if(lastTicks > cticks){lastTicks = cticks;}
		return;
	}
	lastTicks = cticks;

	lEval(clRoot,lCons(lValSym("yield-run"),NULL));

	PROFILE_STOP();
}
