#include "lisp.h"

#include "../main.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../misc/options.h"
#include "../network/chat.h"
#include "../sdl/sdl.h"
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

lClosure *clRoot;
u8 SEvalID;

void lPrintError(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
}

lVal *lispSEvalSym(u8 id){
	static char buf[8];
	snprintf(buf,sizeof(buf)-1,"SEv%03u",id);
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

static lVal *wwlnfThirdPerson(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		optionThirdPerson = t->vInt != 0;
	}

	return lValBool(optionThirdPerson);
}

static lVal *wwlnfFullscreen(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		setFullscreen(t->vInt != 0);
	}

	return lValBool(optionThirdPerson);
}

static lVal *wwlnfDebugInfo(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		optionDebugInfo = t->vInt != 0;
	}

	return lValBool(optionDebugInfo);
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

void addClientNFuncs(lClosure *c){
	lAddNativeFunc(c,"s",            "(...body)","Evaluates ...body on the serverside and returns the last result",wwlnfSEval);
	lAddNativeFunc(c,"player-pos",   "()",       "Returns players position",                                       wwlnfPlayerPos);
	lAddNativeFunc(c,"player-rot",   "()",       "Returns players rotation",                                       wwlnfPlayerVel);
	lAddNativeFunc(c,"player-vel",   "()",       "Returns players velocity",                                       wwlnfPlayerRot);
	lAddNativeFunc(c,"player-name!", "(s)",      "Sets players name to s",                                         wwlnfPlayerName);
	lAddNativeFunc(c,"sound-vol!",   "(f)",      "Sets sound volume to float f",                                   wwlnfSoundVolume);
	lAddNativeFunc(c,"view-dist!",   "(f)",      "Sets render distance to f blocks",                               wwlnfRenderDistance);
	lAddNativeFunc(c,"server-add!",  "(name ip)","Adds name ip to server list",                                    wwlnfServerAdd);
	lAddNativeFunc(c,"third-person!","(b)",      "Sets third person view to b",                                    wwlnfThirdPerson);
	lAddNativeFunc(c,"fullscreen!",  "(b)",      "Sets fullscreen to b",                                           wwlnfFullscreen);
	lAddNativeFunc(c,"save-options", "()",       "Save options to disk",                                           wwlnfSaveOptions);
	lAddNativeFunc(c,"debug-info!",  "(b)",      "Sets debug info view to b",                                      wwlnfDebugInfo);
	lAddNativeFunc(c,"send-message", "(s)",      "Sends string s as a chat message",                               wwlnfSendMessage);
	lAddNativeFunc(c,"console-print","(s)",      "Prints string s to the REPL",                                    wwlnfConsolePrint);
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
	static lVal *expr = NULL;

	if(expr == NULL){
		expr = lWrap(lRead("(yield-run)"));
		expr->flags |= lfNoGC;
	}
	PROFILE_START();

	static uint lastTicks = 0;
	u64 cticks = getTicks();
	if((lastTicks + 500) > cticks){return;}
	lastTicks = cticks;

	lEval(clRoot,expr);
	lClosureGC();

	PROFILE_STOP();
}
