#include "lisp.h"

#include "../../../common/nujel/nujel.h"

lClosure *clRoot;

void lispInit(){
	lInit();
	clRoot = lClosureNew(NULL);
}
void lispFree(){
	lClosureFree(clRoot);
}

const char *lispEval(const char *str){
	static char reply[512];
	*reply = 0;
	lVal *v = NULL;
	for(lVal *sexpr = lParseSExprCS(str); sexpr != NULL; sexpr = sexpr->next){
		v = lEval(clRoot,sexpr);
	}
	lSPrintChain(v,reply,&reply[sizeof(reply)]);
	for(uint i=0;i<sizeof(reply);i++){if(reply[i] == '\n'){reply[i] = ' ';}}
	lClosureGC(clRoot);
	return reply;
}
