#include "lisp.h"

#include "../main.h"
#include "../game/character.h"
#include "../gfx/gfx.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../misc/options.h"
#include "../sdl/sdl.h"
#include "../../../common/src/misc/lisp.h"
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
	lSPrintVal(lWrap(v),buf,&buf[sizeof(buf)-1]);
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
	char *address = "localhost";
	char *name = "localhost";

	if((v != NULL) && (v->type == ltList)){
		lVal *t = lnfCat(c,lEval(c,v->vList.car));
		address = t->vString->buf;
		v = v->vList.cdr;
	}
	if((v != NULL) && (v->type == ltList)){
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


lVal *lResolveNativeSym(const lSymbol s){
	if(strcmp(s.c,"s") == 0)              {return lValNativeFunc(wwlnfSEval);}

	if(strcmp(s.c,"player-pos") == 0)     {return lValNativeFunc(wwlnfPlayerPos);}
	if(strcmp(s.c,"player-vel") == 0)     {return lValNativeFunc(wwlnfPlayerVel);}
	if(strcmp(s.c,"player-rot") == 0)     {return lValNativeFunc(wwlnfPlayerRot);}
	if(strcmp(s.c,"player-name") == 0)    {return lValNativeFunc(wwlnfPlayerName);}
	if(strcmp(s.c,"sound-volume") == 0)   {return lValNativeFunc(wwlnfSoundVolume);}
	if(strcmp(s.c,"render-distance") == 0){return lValNativeFunc(wwlnfRenderDistance);}
	if(strcmp(s.c,"server-add") == 0)     {return lValNativeFunc(wwlnfServerAdd);}
	if(strcmp(s.c,"third-person") == 0)   {return lValNativeFunc(wwlnfThirdPerson);}
	if(strcmp(s.c,"fullscreen") == 0)     {return lValNativeFunc(wwlnfFullscreen);}
	if(strcmp(s.c,"save-options") == 0)   {return lValNativeFunc(wwlnfSaveOptions);}
	if(strcmp(s.c,"debug-info") == 0)     {return lValNativeFunc(wwlnfDebugInfo);}

	return lResolveNativeSymCommon(s);
}

void lispInit(){
	lInit();
	clRoot = lClosureNew(NULL);
	lispEvalNR("(define fasts   (λ (a) (mst  1) (s (mst  1))))");
	lispEvalNR("(define norms   (λ (a) (mst  4) (s (mst  4))))");
	lispEvalNR("(define slows   (λ (a) (mst 16) (s (mst 16))))");
	lispEvalNR("(define bulls   (λ (a) (mst 64) (s (mst 64))))");
	lispEvalNR("(define morning (λ ( ) (s (morning))))");
	lispEvalNR("(define night   (λ ( ) (s (night))))");

}

void lispFree(){
	lClosureFree(clRoot);
}

const char *lispEval(const char *str){
	static char reply[4096];
	memset(reply,0,sizeof(reply));
	lVal *v = lEval(clRoot,lWrap(lRead(str)));
	lSPrintVal(v,reply,&reply[sizeof(reply)-1]);

	int soff,slen,len = strnlen(reply,sizeof(reply)-1);
	for(soff = 0;    isspace(reply[soff]) || (reply[soff] == '"');soff++){}
	for(slen = len-1;isspace(reply[slen]) || (reply[slen] == '"');slen--){reply[slen] = 0;}

	lClosureGC();
	return reply+soff;
}

void lispRecvSExpr(const packet *p){
	u8 id = p->v.u8[0];
	const char *str = (const char *)&p->v.u8[1];
	lispPanelShowReply(lispSEvalSym(id),str);
}
