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
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gfx/texture.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/lispInput.h"
#include "../gui/textInput.h"
#include "../misc/options.h"
#include "../network/chat.h"
#include "../sdl/sdl.h"
#include "../sdl/sfx.h"
#include "../tmp/assets.h"
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
	if(v != NULL){
		lVal *t = lnfCat(c,lEval(c,v));
		if(t->type == ltString){
			strncpy(playerName,t->vString->buf,sizeof(playerName)-1);
			playerName[sizeof(playerName)-1]=0;
		}
	}
	return lValString(playerName);
}

static lVal *wwlnfSoundVolume(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfFloat(c,lEval(c,v));
		optionSoundVolume = t->vFloat;
	}
	return lValFloat(optionSoundVolume);
}

static lVal *wwlnfRenderDistance(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfFloat(c,lEval(c,v));
		setRenderDistance(t->vFloat);
	}
	return lValFloat(renderDistance);
}

static lVal *wwlnfSubData(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfBool(c,lEval(c,v));
		gfxUseSubData = t->vBool;
	}
	return lValBool(gfxUseSubData);
}

static lVal *wwlnfMouseSensitivity(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfFloat(c,lEval(c,v));
		optionMouseSensitivy = t->vFloat;
	}
	return lValFloat(optionMouseSensitivy);
}

static lVal *wwlnfThirdPerson(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfBool(c,lEval(c,v));
		optionThirdPerson = t->vBool;
	}
	return lValBool(optionThirdPerson);
}

static lVal *wwlnfFullscreen(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfBool(c,lEval(c,v));
		setFullscreen(t->vBool);
	}
	return lValBool(optionFullscreen);
}

static lVal *wwlnfWindowed(lClosure *c, lVal *v){
	int args[4] = {800,600,-1,-1};
	for(int i=0;i<4;i++){
		if(v == NULL){break;}
		lVal *t = lnfInt(c,lEval(c,v->vList.car));
		if(t == NULL){break;}
		v = v->vList.cdr;
		args[i] = t->vInt;
	}
	setWindowed(args[0],args[1],args[2],args[3]);
	return NULL;
}

static lVal *wwlnfDebugInfo(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		optionDebugInfo = t->vInt != 0;
	}
	return lValBool(optionDebugInfo);
}

static lVal *wwlnfConsMode(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		if(t->vInt != 0){
			player->flags |=  CHAR_CONS_MODE;
		}else{
			player->flags &= ~CHAR_CONS_MODE;
		}
	}
	return lValBool(player->flags & CHAR_CONS_MODE);
}

static lVal *wwlnfNoClip(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		if(t->vInt != 0){
			player->flags |=  CHAR_NOCLIP;
		}else{
			player->flags &= ~CHAR_NOCLIP;
		}
	}
	return lValBool(player->flags & CHAR_NOCLIP);
}

static lVal *wwlnfWireFrame(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfBool(c,lEval(c,v));
		optionWireframe = t->vBool;
		initGL();
	}
	return lValBool(optionWireframe);
}

static lVal *wwlnfScreenshot(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	queueScreenshot = true;
	return NULL;
}

static lVal *wwlnfSaveOptions(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	saveOptions();
	return lValBool(true);
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
	(void)v;
	(void)c;
	return lValVec(player->pos);
}

static lVal *wwlnfPlayerRot(lClosure *c, lVal *v){
	(void)v;
	(void)c;
	return lValVec(player->rot);
}

static lVal *wwlnfPlayerVel(lClosure *c, lVal *v){
	(void)v;
	(void)c;
	return lValVec(player->vel);
}

static lVal *wwlnfFireHook(lClosure *c, lVal *v){
	(void)v;
	(void)c;
	characterFireHook(player);
	return NULL;
}

static lVal *wwlnfInvActiveSlot(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		player->activeItem = t->vInt;
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
	int sfxID = -1;
	float volume = 1.0;
	vec pos = player->pos;

	for(int i=0;i<3;i++){
		if(v == NULL){break;}
		lVal *t = lEval(c,v->vList.car);
		v = v->vList.cdr;
		if(t == NULL){continue;}
		switch(i){
		case 0:
			t = lnfInt(c,t);
			sfxID = t->vInt;
			break;
		case 1:
			t = lnfFloat(c,t);
			volume = t->vFloat;
			break;
		case 2:
			t = lnfVec(c,t);
			pos = t->vVec;
			break;
		}
	}
	if(sfxID < 0){return lValBool(false);}
	sfxPlayPos(&sfxList[sfxID],volume,pos);
	return lValBool(true);
}

static lVal *wwlnfReloadTextures(lClosure *c, lVal *v){
	(void)v;
	(void)c;
	textureReload();
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

void addClientNFuncs(lClosure *c){
	lAddNativeFunc(c,"s",              "(...body)",    "Evaluates ...body on the serverside and returns the last result",wwlnfSEval);
	lAddNativeFunc(c,"text-focus?",    "()",           "Returns if a text input field is currently focused",             wwlnfTextInputFocusPred);
	lAddNativeFunc(c,"player-pos",     "()",           "Returns players position",                                       wwlnfPlayerPos);
	lAddNativeFunc(c,"player-rot",     "()",           "Returns players rotation",                                       wwlnfPlayerVel);
	lAddNativeFunc(c,"player-vel",     "()",           "Returns players velocity",                                       wwlnfPlayerRot);
	lAddNativeFunc(c,"player-name!",   "(s)",          "Sets players name to s",                                         wwlnfPlayerName);
	lAddNativeFunc(c,"sound-vol!",     "(f)",          "Sets sound volume to float f",                                   wwlnfSoundVolume);
	lAddNativeFunc(c,"view-dist!",     "(f)",          "Sets render distance to f blocks",                               wwlnfRenderDistance);
	lAddNativeFunc(c,"use-sub-data!",  "(b)",          "Sets useSubdata boolean to b",                                   wwlnfSubData);
	lAddNativeFunc(c,"mouse-sens!",    "(f)",          "Sets the mouse sensitivity to f",                                wwlnfMouseSensitivity);
	lAddNativeFunc(c,"server-add!",    "(name ip)",    "Adds name ip to server list",                                    wwlnfServerAdd);
	lAddNativeFunc(c,"third-person!",  "(b)",          "Sets third person view to b",                                    wwlnfThirdPerson);
	lAddNativeFunc(c,"fullscreen",     "(b)",          "Sets fullscreen to b",                                           wwlnfFullscreen);
	lAddNativeFunc(c,"windowed",       "(&w &h &x &y)","Switches to windowed mode of size &w/&h at position &x/&y",      wwlnfWindowed);
	lAddNativeFunc(c,"save-options",   "()",           "Save options to disk",                                           wwlnfSaveOptions);
	lAddNativeFunc(c,"texture-reload", "()",           "Reloads all textures from disk",                                 wwlnfReloadTextures);
	lAddNativeFunc(c,"reset-worst-f",  "()",           "Resets the worst frame counter",                                 wwlnfResetWorstFrame);
	lAddNativeFunc(c,"debug-info!",    "(b)",          "Sets debug info view to b",                                      wwlnfDebugInfo);
	lAddNativeFunc(c,"cons-mode!",     "(b)",          "Sets cons-mode to b if passed, always returns the current state",wwlnfConsMode);
	lAddNativeFunc(c,"no-clip!",       "(b)",          "Sets no clip to b if passed, always returns the current state",  wwlnfNoClip);
	lAddNativeFunc(c,"wire-frame!",    "(b)",          "Sets wireframe mode to b, always returns the current state",     wwlnfWireFrame);
	lAddNativeFunc(c,"send-message",   "(s)",          "Sends string s as a chat message",                               wwlnfSendMessage);
	lAddNativeFunc(c,"console-print",  "(s)",          "Prints string s to the REPL",                                    wwlnfConsolePrint);
	lAddNativeFunc(c,"sfx-play",       "(s &vol &pos)","Plays SFX s with volume &vol as if emitting from &pos.",         wwlnfSfxPlay);
	lAddNativeFunc(c,"screenshot",     "()",           "Takes a screeshot",                                              wwlnfScreenshot);
	lAddNativeFunc(c,"fire-hook",      "()",           "Fires the players Grappling hook, or retracts it if fired",      wwlnfFireHook);
	lAddNativeFunc(c,"inv-active-slot","(i)",          "Sets the players active item to i",                              wwlnfInvActiveSlot);
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
