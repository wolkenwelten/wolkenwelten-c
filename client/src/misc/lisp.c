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
#include "../binding/widget.h"
#include "../binding/widgetGC.h"
#include "../game/beamblast.h"
#include "../game/character/character.h"
#include "../game/character/hook.h"
#include "../game/fire.h"
#include "../game/grenade.h"
#include "../game/projectile.h"
#include "../game/recipe.h"
#include "../game/throwable.h"
#include "../game/weather/weather.h"
#include "../gfx/boundaries.h"
#include "../gfx/gfx.h"
#include "../gfx/texture.h"
#include "../gui/chat.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/menu/inventory.h"
#include "../gui/repl.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../misc/options.h"
#include "../network/chat.h"
#include "../network/client.h"
#include "../sdl/sdl.h"
#include "../sfx/sfx.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/lisp.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include "../../../common/nujel/lib/api.h"
#include "../../../common/nujel/lib/nujel.h"
#include "../../../common/nujel/lib/exception.h"
#include "../../../common/nujel/lib/allocation/roots.h"
#include "../../../common/nujel/lib/misc/pf.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

u8 SEvalID;

extern unsigned  int src_tmp_client_nuj_len;
extern unsigned char src_tmp_client_nuj_data[];

void lispInputHandler(lSymbol *input, int key, int action){
	const int SP = lRootsGet();
	lVal *form = RVP(lList(3,RVP(lValSymS(input)),RVP(lValInt(action)),RVP(lValInt(key))));
	lExceptionTryExit(lispCallFuncReal,clRoot,form);
	lRootsRet(SP);
}

void lispInputTick(){
	static lVal *form = NULL;
	if(form == NULL){
		form = RVP(lCons(RVP(lValSym("input-tick")),NULL));
	}
	const int SP = lRootsGet();
	lExceptionTryExit(lispCallFuncReal,clRoot,form);
	lRootsRet(SP);
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

static void *lispEvalNRReal(void *a, void *b){
	(void)b;
	const char *str = a;
	lVal *expr = lRead(str);
	return lnfDo(clRoot,expr);
}

lVal *lispEvalNR(const char *str){
	return lExceptionTryExit(lispEvalNRReal,(void *)str, NULL);
}

static lVal *wwlnfSEval(lClosure *c, lVal *v){
	(void)c;
	char buf[1<<14];
	memset(buf,0,sizeof(buf));
	const int SP = lRootsGet();
	spf(buf, &buf[sizeof(buf)-1], "%V", lCar(v));
	msgLispSExpr(-1,buf);
	lRootsRet(SP);
	return NULL;
}

static lVal *wwlnfPlayerName(lClosure *c, lVal *v){
	(void)c;
	const char *npName = castToString(lCar(v),NULL);

	if(npName != NULL){
		snprintf(playerName,sizeof(playerName),"%s",npName);
	}
	return lValString(playerName);
}

static lVal *wwlnfSoundVolume(lClosure *c, lVal *v){
	(void)c;
	const float nvol = castToFloat(lCar(v),-1.f);
	if(nvol > -0.1f){optionSoundVolume = nvol;}
	return lValFloat(optionSoundVolume);
}

static lVal *wwlnfRenderDistance(lClosure *c, lVal *v){
	(void)c;
	const float rdist = castToFloat(lCar(v),0.f);
	if(rdist > 1.f){ setRenderDistance(rdist); }
	return lValFloat(renderDistance);
}

static lVal *wwlnfSubData(lClosure *c, lVal *v){
	(void)c;
	if((v != NULL) && (v->type == ltPair)){
		gfxUseSubData = castToBool(lCar(v));
	}
	return lValBool(gfxUseSubData);
}

static lVal *wwlnfMouseSensitivity(lClosure *c, lVal *v){
	(void)c;
	const float msen = castToFloat(lCar(v),0.f);
	if(msen > 0.f){ optionMouseSensitivy = msen; }
	return lValFloat(optionMouseSensitivy);
}

static lVal *wwlnfThirdPerson(lClosure *c, lVal *v){
	(void)c;
	if((v != NULL) && (v->type == ltPair)){
		optionThirdPerson = castToBool(lCar(v));
	}
	return lValBool(optionThirdPerson);
}

static lVal *wwlnfFullscreen(lClosure *c, lVal *v){
	(void)c;
	if((v != NULL) && (v->type == ltPair)){
		setFullscreen(castToBool(lCar(v)));
	}
	return NULL;
}

static lVal *wwlnfFullscreenPred(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValBool(optionFullscreen);
}

static lVal *wwlnfWindowed(lClosure *c, lVal *v){
	(void)c;
	const int w = castToInt(lCar(v),800); v = lCdr(v);
	const int h = castToInt(lCar(v),600); v = lCdr(v);
	const int x = castToInt(lCar(v), -1); v = lCdr(v);
	const int y = castToInt(lCar(v), -1);

	setWindowed(w,h,x,y);
	return NULL;
}

static lVal *wwlnfDebugInfoGet(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValBool(optionDebugInfo);
}

static lVal *wwlnfDebugInfoSet(lClosure *c, lVal *v){
	(void)c; (void)v;
	optionDebugInfo = castToBool(lCar(v));
	return NULL;
}

static lVal *wwlnfConsModeGet(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValBool(player->flags & CHAR_CONS_MODE);
}

static lVal *wwlnfConsModeSet(lClosure *c, lVal *v){
	(void)c;
	if(castToBool(lCar(v))){
		player->flags |=  CHAR_CONS_MODE;
	}else{
		player->flags &= ~CHAR_CONS_MODE;
	}
	return NULL;
}

static lVal *wwlnfNoClipGet(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValBool(player->flags & CHAR_NOCLIP);
}

static lVal *wwlnfNoClipSet(lClosure *c, lVal *v){
	(void)c; (void)v;
	if(castToBool(lCar(v))){
		player->flags |=  CHAR_NOCLIP;
	}else{
		player->flags &= ~CHAR_NOCLIP;
	}
	return NULL;
}

static lVal *wwlnfWireFrameGet(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValBool(optionWireframe);
}

static lVal *wwlnfWireFrameSet(lClosure *c, lVal *v){
	(void)c;
	optionWireframe = castToBool(lCar(v));
	initGL();
	return NULL;
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
	(void)c;
	const char *address = castToString(lCar(v),"localhost"); v = lCdr(v);
	const char *name =    castToString(lCar(v),"Local");
	serverListAdd(address,name);

	return lValFloat(renderDistance);
}

static lVal *wwlnfPlayerPos(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValVec(player->pos);
}

static lVal *wwlnfPlayerRot(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValVec(player->rot);
}

static lVal *wwlnfPlayerVel(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValVec(player->vel);
}

static lVal *wwlnfFireHook(lClosure *c, lVal *v){
	(void)c;(void)v;
	characterFireHook(player);
	return NULL;
}

static lVal *wwlnfPlayerActiveSlotGet(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(player->activeItem);
}

static lVal *wwlnfSetCooldown(lClosure *c, lVal *v){
	(void)c;
	if(player == NULL){return NULL;}
	const int cd = castToInt(lCar(v),0);

	characterSetCooldown(player,cd);

	return lValInt(player->actionTimeout);
}

static lVal *wwlnfPlayerActiveSlotSet(lClosure *c, lVal *v){
	(void)c;
	const int ai = castToInt(lCar(v),-1);
	if(player == NULL){return NULL;}
	if(ai >= 0){
		player->activeItem = ai;
		player->flags &= ~(CHAR_AIMING | CHAR_THROW_AIM);
		player->goalZoomFactor = 1.f;
	}
	return NULL;
}

static lVal *wwlnfSendMessage(lClosure *c, lVal *v){
	(void)c;
	const char *msg = castToString(lCar(v),NULL);
	if(msg == NULL){return NULL;}
	chatSend(msg);
	return lCar(v);
}

static lVal *wwlnfConsolePrint(lClosure *c, lVal *v){
	(void)c;
	const char *msg = castToString(lCar(v),NULL);
	if(msg == NULL){return NULL;}
	widgetAddEntry(lispLog, msg);
	return lCar(v);
}

static lVal *wwlnfSfxPlay(lClosure *c, lVal *v){
	(void)c;
	const int sfxID    = castToInt(lCar(v),-1);    v = lCdr(v);
	const float volume = castToFloat(lCar(v),1.0); v = lCdr(v);
	const vec pos      = castToVec(lCar(v),player->pos);

	if(sfxID < 0){return NULL;}
	sfxPlayPos(&sfxList[sfxID],volume,pos);
	return NULL;
}

static lVal *wwlnfResetWorstFrame(lClosure *c, lVal *v){
	(void)v; (void)c;
	worstFrame = 0;
	return NULL;
}

static lVal *wwlnfTextInputFocusPred(lClosure *c, lVal *v){
	(void)v; (void)c;
	return lValBool(textInputActive());
}

static lVal *wwlnfServerExecutable(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValString(clientGetServerExecutable());
}

static lVal *wwlnfTryToUse(lClosure *c, lVal *v){
	(void)c;
	const int ms     = castToInt(lCar(v),800); v = lCdr(v);
	const int amount = castToInt(lCar(v),1);
	item *itm  = &player->inventory[player->activeItem];

	bool ret = characterTryToUse(player,itm,ms,amount);
	return lValBool(ret);
}

static lVal *wwlnfStartAnim(lClosure *c, lVal *v){
	(void)c;
	const int anim = castToInt(lCar(v),0); v = lCdr(v);
	const int ms =   castToInt(lCar(v),200);

	characterStartAnimation(player,anim,ms);
	return NULL;
}

static lVal *wwlnfStopAnim(lClosure *c, lVal *v){
	(void)c;(void)v;
	characterStopAnimation(player);
	return NULL;
}

static lVal *wwlnfGrenadeNew(lClosure *c, lVal *v){
	(void)c;
	const vec pos          = castToVec(lCar(v),player->pos); v = lCdr(v);
	const vec rot          = castToVec(lCar(v),player->rot); v = lCdr(v);
	const float pwr        = castToFloat(lCar(v),4.f);       v = lCdr(v);
	const int cluster      = castToInt(lCar(v),0);           v = lCdr(v);
	const float clusterPwr = castToFloat(lCar(v),0.f);

	grenadeNew(pos,rot,pwr,cluster,clusterPwr);
	return NULL;
}

static lVal *wwlnfTryToThrow(lClosure *c, lVal *v){
	(void)c;(void)v;
	item *itm = &player->inventory[player->activeItem];
	return lValBool(throwableTryAim(itm,player));
}

static lVal *wwlnfItemReload(lClosure *c, lVal *v){
	(void)c;
	const int ms = castToInt(lCar(v),200);
	item *itm = &player->inventory[player->activeItem];

	characterItemReload(player,itm,ms);
	return NULL;
}

static lVal *wwlnfToggleAim(lClosure *c, lVal *v){
	(void)c;
	const float zoom = castToFloat(lCar(v),2.f);

	characterToggleAim(player,zoom);
	return NULL;
}

static lVal *wwlnfInaccuracy(lClosure *c, lVal *v){
	(void)c;
	const float inacc = castToFloat(lCar(v),-1024.f);
	if(inacc > -1024.f){characterSetInaccuracy(player,inacc);}
	return lValFloat(player->inaccuracy);
}

static lVal *wwlnfRecoil(lClosure *c, lVal *v){
	(void)c;
	const float recoil = castToFloat(lCar(v),1.f);
	characterAddRecoil(player,recoil);
	return NULL;
}

static lVal *wwlnfPlayerHP(lClosure *c, lVal *v){
	(void)c;
	const int hp = castToInt(lCar(v),-1024);
	if(hp > -1024){ player->hp = MIN(player->maxhp,hp); }
	return lValInt(player->hp);
}

static lVal *wwlnfPlayerMaxHP(lClosure *c, lVal *v){
	(void)c;
	const int maxhp = castToInt(lCar(v),-1024);
	if(maxhp > -1024){ player->maxhp = MAX(1,maxhp); }
	return lValInt(player->maxhp);
}

static lVal *wwlnfRResult(lClosure *c, lVal *v){
	(void)c;
	const int id     = castToInt(lCar(v),-1); v = lCdr(v);
	const int result = castToInt(lCar(v),0);  v = lCdr(v);
	const int amount = castToInt(lCar(v),0);

	if((id < 0) || (result <= 0) || (amount <= 0)){return NULL;}
	recipeCount = MAX(id+1,(int)recipeCount);
	recipes[id].result.ID = result;
	recipes[id].result.amount = amount;
	return NULL;
}

static lVal *wwlnfRIngred(lClosure *c, lVal *v){
	(void)c;
	const int id     = castToInt(lCar(v),-1); v = lCdr(v);
	const int ii     = castToInt(lCar(v),-1); v = lCdr(v);
	const int ingred = castToInt(lCar(v),0);  v = lCdr(v);
	const int amount = castToInt(lCar(v),0);

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
	(void)c;
	const int flags   = castToInt(lCar(v),0);      v = lCdr(v);
	const float force = castToFloat(lCar(v),0.1f); v = lCdr(v);
	const int damage  = castToInt(lCar(v),1);

	item *itm = &player->inventory[player->activeItem];
	bool ret  = throwableTry(itm,player,force,damage,flags);
	return lValBool(ret);
}

static lVal *wwlnfTryToShoot(lClosure *c, lVal *v){
	(void)c;
	const int cooldown    = castToInt(lCar(v),800); v = lCdr(v);
	const int bulletcount = castToInt(lCar(v),1);

	item *itm = &player->inventory[player->activeItem];
	bool ret  = characterTryToShoot(player,itm,cooldown,bulletcount);
	return lValBool(ret);
}

static lVal *wwlnfBeamblast(lClosure *c, lVal *v){
	(void)c;
	const float beamSize = castToFloat(lCar(v),1.f); v = lCdr(v);
	const float damage   = castToFloat(lCar(v),8.f); v = lCdr(v);
	const int hitsLeft   = castToInt(lCar(v),3);

	beamblast(player,beamSize,damage,hitsLeft);
	return NULL;
}

static lVal *wwlnfProjectile(lClosure *c, lVal *v){
	(void)c;
	const int type = castToInt(lCar(v),0); v = lCdr(v);
	const int num  = castToInt(lCar(v),1);

	projectileNewC(player,type,num);
	return NULL;
}

static lVal *wwlnfRaycast(lClosure *c, lVal *v){
	(void)c;
	bool before = castToBool(lCar(v));

	return lValVec(characterLOSBlock(player,before));
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
	characterMineStop(player);
	return NULL;
}

static lVal *wwlnfPlayerInventoryGet(lClosure *c, lVal *v){
	(void)c;
	const int slot = castToInt(lCar(v),-1);

	if(slot < 0)                               {return NULL;}
	if(slot >= (int)countof(player->inventory)){return NULL;}
	const item *itm = &player->inventory[slot];
	return lCons(lValInt(itm->ID),lValInt(itm->amount));

}
static lVal *wwlnfPlayerInventorySet(lClosure *c, lVal *v){
	(void)c;
	const int slot   = castToInt(lCar(v),-1); v = lCdr(v);
	const int itemID = castToInt(lCar(v),-1); v = lCdr(v);
	const int amount = castToInt(lCar(v),-1);

	if(slot < 0)                               {return NULL;}
	if(slot >= (int)countof(player->inventory)){return NULL;}
	if((itemID > 0) && (amount > 0)){
		player->inventory[slot] = itemNew(itemID,amount);
	}
	return NULL;
}

static lVal *wwlnfPlayerInventorySizeGet(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInt(player->inventorySize);
}

static lVal *wwlnfIngredientAmountGet(lClosure *c, lVal *v){
	(void)c;
	const int id = castToInt(lCar(v),-1);
	if(id <= 0){
		return lValInt(0);
	}else{
		return lValInt(characterGetItemOrSubstituteAmount(player,id));
	}
}

static lVal *wwlnfIngredientAmountDecrement(lClosure *c, lVal *v){
	(void)c;
	const int id  = castToInt(lCar(v),-1); v = lCdr(v);
	const int amt = castToInt(lCar(v),-1);
	if(id <= 0){
		return lValInt(0);
	}else{
		return lValInt(characterDecItemOrSubstituteAmount(player, id, amt));
	}
}

static lVal *wwlnfPlayerPlaceBlock(lClosure *c, lVal *v){
	(void)c;
	const int slot = castToInt(lCar(v),-1);

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
	(void)c;
	const float zoom = MINMAX(castToFloat(lCar(v),-1.0),1.f,8.f);
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
	(void)c;
	const int slot = castToInt(lCar(v),-1);

	if(slot < 0)                               {return NULL;}
	if(slot >= (int)countof(player->inventory)){return NULL;}
	characterItemDropSingle(player,slot);

	return NULL;
}

static lVal *wwlnfPlayerJump(lClosure *c, lVal *v){
	(void)c;
	player->controls.y = castToFloat(lCar(v),0.f);

	return NULL;
}

static lVal *wwlnfPlayerSneak(lClosure *c, lVal *v){
	(void)c;
	if(castToBool(lCar(v))){
		player->flags |=  CHAR_SNEAK;
	}else{
		player->flags &= ~CHAR_SNEAK;
	}

	return NULL;
}

static lVal *wwlnfPlayerWalk(lClosure *c, lVal *v){
	(void)c;
	player->controls.z = castToFloat(lCar(v),0.f);

	return NULL;
}

static lVal *wwlnfPlayerStrafe(lClosure *c, lVal *v){
	(void)c;
	player->controls.x = castToFloat(lCar(v),0.f);

	return NULL;
}

static lVal *wwlnfToggleInventory(lClosure *c, lVal *v){
	(void)c;(void)v;
	toggleInventory();
	return NULL;
}

static lVal *wwlnfPlayerBoost(lClosure *c, lVal *v){
	(void)c;
	if(castToBool(lCar(v))){
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

static lVal *wwlnfPlayerGetFlags(lClosure *c, lVal *v){
	(void)c;(void)v;
	if(player == NULL){return NULL;}
	return lValInt(player->flags);
}

static lVal *wwlnfPlayerSetFlags(lClosure *c, lVal *v){
	(void)c;
	if(player == NULL){return NULL;}
	player->flags = castToInt(lCar(v),0);
	return NULL;
}

static lVal *wwlnfDrawBoundariesGet(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(drawBoundariesStyle);
}

static lVal *wwlnfDrawBoundariesSet(lClosure *c, lVal *v){
	(void)c;
	const int newStyle = castToInt(lCar(v),-1);
	if(newStyle >= 0){
		drawBoundariesStyle = newStyle;
	}
	return NULL;
}

static lVal *wwlnfChatOpenGet(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValBool(chatIsOpen());
}

static lVal *wwlnfChatOpenSet(lClosure *c, lVal *v){
	(void)c;
	if(castToBool(lCar(v))){
		chatOpen();
	}else{
		chatClose();
	}
	return NULL;
}

static void lispAddClientNFuncs(lClosure *c){
	lOperatorsWidget(c);

	lAddNativeFunc(c,"s*",             "(form)",            "Evaluates form on the server and returns the last result",       wwlnfSEval);
	lAddNativeFunc(c,"text-focus?",    "()",                "Returns if a text input field is currently focused",             wwlnfTextInputFocusPred);
	lAddNativeFunc(c,"player-pos",     "()",                "Return players position",                                        wwlnfPlayerPos);
	lAddNativeFunc(c,"player-rot",     "()",                "Return players rotation",                                        wwlnfPlayerRot);
	lAddNativeFunc(c,"player-vel",     "()",                "Return players velocity",                                        wwlnfPlayerVel);
	lAddNativeFunc(c,"player-flags",   "()",                "Return players flags",                                           wwlnfPlayerGetFlags);
	lAddNativeFunc(c,"player-flags!",  "(flags)",           "Set players flags",                                              wwlnfPlayerSetFlags);
	lAddNativeFunc(c,"player-name!",   "(s)",               "Set players name to s",                                          wwlnfPlayerName);
	lAddNativeFunc(c,"player-hp",      "(&hp)",             "Set the players health to &HP, returns the current value.",      wwlnfPlayerHP);
	lAddNativeFunc(c,"player-maxhp",   "(&mhp)",            "Set the players max health to &MHP, returns the current value.", wwlnfPlayerMaxHP);
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
	lAddNativeFunc(c,"fullscreen?",    "()",                "#t if Fullscreen",                                               wwlnfFullscreenPred);
	lAddNativeFunc(c,"fullscreen!",    "(b)",               "Sets fullscreen to b",                                           wwlnfFullscreen);
	lAddNativeFunc(c,"windowed",       "(&width &height &x &y)",     "Switches to windowed mode of size WIDTH/HEIGHT at position X/Y", wwlnfWindowed);
	lAddNativeFunc(c,"window-width",   "()",                "Returns the width of the widow in pixels",                       wwlnfWindowWidth);
	lAddNativeFunc(c,"window-height",  "()",                "Returns the height of the widow in pixels",                      wwlnfWindowHeight);
	lAddNativeFunc(c,"save-options",   "()",                "Save options to disk",                                           wwlnfSaveOptions);
	lAddNativeFunc(c,"reset-worst-f",  "()",                "Resets the worst frame counter",                                 wwlnfResetWorstFrame);
	lAddNativeFunc(c,"debug-info!",    "(b)",               "Sets debug info view to b",                                      wwlnfDebugInfoSet);
	lAddNativeFunc(c,"debug-info?",    "()",                "Gets debug info view to b",                                      wwlnfDebugInfoGet);
	lAddNativeFunc(c,"cons-mode!",     "(b)",               "Sets cons-mode to b if passed, always returns the current state",wwlnfConsModeSet);
	lAddNativeFunc(c,"cons-mode?",     "()",                "Gets cons-mode to b if passed, always returns the current state",wwlnfConsModeGet);
	lAddNativeFunc(c,"no-clip!",       "(b)",               "Sets no clip to b if passed, always returns the current state",  wwlnfNoClipSet);
	lAddNativeFunc(c,"no-clip?",       "()",                "Gets no clip to b if passed, always returns the current state",  wwlnfNoClipGet);
	lAddNativeFunc(c,"wire-frame!",    "(b)",               "Sets wireframe mode to b, always returns the current state",     wwlnfWireFrameSet);
	lAddNativeFunc(c,"wire-frame?",    "()",                "Sets wireframe mode to b, always returns the current state",     wwlnfWireFrameGet);
	lAddNativeFunc(c,"send-message",   "(s)",               "Sends string s as a chat message",                               wwlnfSendMessage);
	lAddNativeFunc(c,"console-print",  "(s)",               "Prints string s to the REPL",                                    wwlnfConsolePrint);
	lAddNativeFunc(c,"sfx-play",       "(s &vol &pos)",     "Plays SFX S with volume &VOL=1.0 as if emitting from &POS.",     wwlnfSfxPlay);
	lAddNativeFunc(c,"screenshot",     "()",                "Takes a screeshot",                                              wwlnfScreenshot);
	lAddNativeFunc(c,"fire-hook",      "()",                "Fires the players Grappling hook, or retracts it if fired",      wwlnfFireHook);
	lAddNativeFunc(c,"player-active-slot", "()",            "Get the players active slot",                                    wwlnfPlayerActiveSlotGet);
	lAddNativeFunc(c,"player-active-slot!","(slot)",        "Set the players active SLOT",                                    wwlnfPlayerActiveSlotSet);
	lAddNativeFunc(c,"player-inventory",   "(slot)",           "Get the players inventory SLOT",                              wwlnfPlayerInventoryGet);
	lAddNativeFunc(c,"player-inventory!",  "(slot id &amount)","Set the players inventory SLOT to ID and AMOUNT",             wwlnfPlayerInventorySet);
	lAddNativeFunc(c,"player-inventory-size","()",          "Return the size of the players inventory",                       wwlnfPlayerInventorySizeGet);
	lAddNativeFunc(c,"player-ingredient-get","(id)",        "Return the amount of items the player has of ID and its substitutes",wwlnfIngredientAmountGet);
	lAddNativeFunc(c,"player-ingredient-decrement","(id amount)","Remove AMOUNT items of ID and its substitutes",             wwlnfIngredientAmountDecrement);
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
	lAddNativeFunc(c,"player-drop-item!","(slot)",          "Throw a single item from SLOT in front of the player",           wwlnfDropItem);
	lAddNativeFunc(c,"try-to-shoot",   "(cd ammo)",         "Try to shoot and cooldown for CD and use AMMO bullets",          wwlnfTryToShoot);
	lAddNativeFunc(c,"beamblast",      "(size damage hits-left)", "Calls cFunc beamblast",                                    wwlnfBeamblast);
	lAddNativeFunc(c,"projectile",     "(&type &num)",      "Fire &NUM=1 projectiles of &TYPE=0",                             wwlnfProjectile);
	lAddNativeFunc(c,"toggle-inventory!","()",              "Toggle the inveotory",                                           wwlnfToggleInventory);
	lAddNativeFunc(c,"widget-focus-on-game?","()",          "Return #t if the game is focused and not some menu",             wwlnfGuiFocusOnGame);
	lAddNativeFunc(c,"draw-boundaries", "()",                "Return the current boundary drawing style",                     wwlnfDrawBoundariesGet);
	lAddNativeFunc(c,"draw-boundaries!","(v)",               "Set the current boundary drawing style",                        wwlnfDrawBoundariesSet);
	lAddNativeFunc(c,"chat-open", "()",                      "Return #t if the chat is open right now",                       wwlnfChatOpenGet);
	lAddNativeFunc(c,"chat-open!","(v)",                     "Opens/Closes the chat",                                         wwlnfChatOpenSet);
	lAddNativeFunc(c,"set-cooldown", "(cd)",                 "Add CD to the current cooldown timer",                          wwlnfSetCooldown);
}

void *lispInitReal(void *a, void *b){
	(void)a; (void)b;

	clRoot = lispCommonRoot(lispAddClientNFuncs);
	lLoadS(clRoot, (const char *)src_tmp_client_nuj_data, src_tmp_client_nuj_len);

	return NULL;
}

void lispInit(){
	widgetGCInit();
	lExceptionTryExit(lispInitReal,NULL,NULL);
}

void lispFree(){
	lClosureFree(clRoot);
}

const char *lispEval(const char *str, bool humanReadable){
	static char reply[4096];
	memset(reply,0,sizeof(reply));

	lVal *v = lispCallFuncS("repl/console",str);
	spf(reply,&reply[sizeof(reply)-1],humanReadable ? "%v" : "%V", v);
	return reply;
}

lVal *lispEvalL(lVal *expr){
	return lExceptionTryExit(lispCallFuncReal,clRoot,expr);
}

void lispRecvSExpr(const packet *p){
	lispPanelShowReply((const char *)&p->v.u8);
}

static void lispGameplay(){
	static lVal *form = NULL;
	if(form == NULL){
		form = lRootsValPush(lCons(NULL,NULL));
		form->vList.car = lValSym("gameplay-run");
	}

	static u64 lastTicks = 0;
	u64 cticks = getTicks();
	if((lastTicks + 1000) > cticks){
		if(lastTicks > cticks){lastTicks = cticks;}
		return;
	}
	lastTicks = cticks;

	const int SP = lRootsGet();
	lExceptionTryExit(lispCallFuncReal,clRoot,form);
	lRootsRet(SP);
}

void lispEvents(){
	PROFILE_START();
	static lVal *form = NULL;
	if(form == NULL){
		form = lRootsValPush(lCons(NULL,NULL));
		form->vList.car = lValSym("yield-run");
	}

	static u64 lastTicks = 0;
	u64 cticks = getTicks();
	if((lastTicks + 50) > cticks){
		if(lastTicks > cticks){lastTicks = cticks;}
		return;
	}
	lastTicks = cticks;

	const int SP = lRootsGet();
	lExceptionTryExit(lispCallFuncReal,clRoot,form);
	lRootsRet(SP);
	lispGameplay();

	PROFILE_STOP();
}
