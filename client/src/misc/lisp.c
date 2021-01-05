#include "lisp.h"

#include "../main.h"
#include "../gfx/gfx.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../misc/options.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"
#include "../../../common/src/nujel/nujel.h"
#include "../../../common/src/nujel/arithmetic.h"
#include "../../../common/src/nujel/string.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

lClosure *clRoot;
u8 SEvalID;

lVal *lispSEvalSym(u8 id){
	static char buf[8];
	snprintf(buf,sizeof(buf)-1,"SEv%03u",id);
	return lValSym(buf);
}

void lispEvalNR(const char *str){
	for(lVal *sexpr = lParseSExprCS(str); sexpr != NULL; sexpr = sexpr->next){
		lEval(clRoot,sexpr);
	}
}

lVal *wwlnfSEval(lClosure *c, lVal *v){
	(void)c;
	static char buf[1024];
	memset(buf,0,sizeof(buf));
	lSPrintChain(v,buf,&buf[sizeof(buf)-1]);
	if(++SEvalID == 0){++SEvalID;}
	msgLispSExpr(-1,SEvalID,buf);
	return lispSEvalSym(SEvalID);
}

lVal *wwlnfMsPerTick(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v));
		if(t->vInt > 0){
			msPerTick = t->vInt;
		}
	}
	return lValInt(msPerTick);
}

lVal *wwlnfProf(lClosure *c, lVal *v){
	(void)c;
	(void)v;

	return lValString(profGetReport());
}

lVal *wwlnfPlayerName(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfCat(c,lEval(c,v));
		if(t->type == ltString){
			strncpy(playerName,t->vString->buf,sizeof(playerName)-1);
			playerName[sizeof(playerName)-1]=0;
		}
	}

	return lValString(playerName);
}

lVal *wwlnfSoundVolume(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfFloat(c,lEval(c,v));
		optionSoundVolume = t->vFloat;
	}

	return lValFloat(optionSoundVolume);
}

lVal *wwlnfRenderDistance(lClosure *c, lVal *v){
	if(v != NULL){
		lVal *t = lnfFloat(c,lEval(c,v));
		setRenderDistance(t->vFloat);
	}

	return lValFloat(renderDistance);
}

lVal *wwlnfServerAdd(lClosure *c, lVal *v){
	char *address = "localhost";
	char *name = "localhost";

	if(v != NULL){
		lVal *t = lnfCat(c,lEval(c,v));
		address = t->vString->buf;
		v = v->next;
	}
	if(v != NULL){
		lVal *t = lnfCat(c,lEval(c,v));
		name = t->vString->buf;
	}

	serverListAdd(address,name);

	return lValFloat(renderDistance);
}

lVal *lResolveNativeSym(const lSymbol s){
	if(strcmp(s.c,"s") == 0)              {return lValNativeFunc(wwlnfSEval);}
	if(strcmp(s.c,"mst") == 0)            {return lValNativeFunc(wwlnfMsPerTick);}
	if(strcmp(s.c,"prof") == 0)           {return lValNativeFunc(wwlnfProf);}
	if(strcmp(s.c,"player-name") == 0)    {return lValNativeFunc(wwlnfPlayerName);}
	if(strcmp(s.c,"sound-volume") == 0)   {return lValNativeFunc(wwlnfSoundVolume);}
	if(strcmp(s.c,"render-distance") == 0){return lValNativeFunc(wwlnfRenderDistance);}
	if(strcmp(s.c,"server-add") == 0)     {return lValNativeFunc(wwlnfServerAdd);}

	return lResolveNativeSymBuiltin(s);
}

void lispInit(){
	lInit();
	clRoot = lClosureNew(NULL);
	lispEvalNR("(define fasts (λ (a) (mst  1) (s (mst  1))))");
	lispEvalNR("(define norms (λ (a) (mst  4) (s (mst  4))))");
	lispEvalNR("(define slows (λ (a) (mst 16) (s (mst 16))))");
	lispEvalNR("(define bulls (λ (a) (mst 64) (s (mst 64))))");

}

void lispFree(){
	lClosureFree(clRoot);
}

const char *lispEval(const char *str){
	static char reply[4096];
	memset(reply,0,sizeof(reply));
	lVal *v = NULL;
	for(lVal *sexpr = lParseSExprCS(str); sexpr != NULL; sexpr = sexpr->next){
		v = lEval(clRoot,sexpr);
	}
	lSPrintChain(v,reply,&reply[sizeof(reply)-1]);

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
