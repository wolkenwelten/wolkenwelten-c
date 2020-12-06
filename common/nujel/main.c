#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "nujel.h"

char *ansiRS = "\033[0m";
char *ansiFG[16] = {
	"\033[0;30m",
	"\033[0;31m",
	"\033[0;32m",
	"\033[0;33m",
	"\033[0;34m",
	"\033[0;35m",
	"\033[0;36m",
	"\033[0;37m",
	"\033[1;30m",
	"\033[1;31m",
	"\033[1;32m",
	"\033[1;33m",
	"\033[1;34m",
	"\033[1;35m",
	"\033[1;36m",
	"\033[1;37m"
};

void *loadFile(const char *filename,size_t *len){
	FILE *fp;
	size_t filelen,readlen,read;
	u8 *buf = NULL;

	fp = fopen(filename,"rb");
	if(fp == NULL){return NULL;}

	fseek(fp,0,SEEK_END);
	filelen = ftell(fp);
	fseek(fp,0,SEEK_SET);

	buf = malloc(filelen+1);
	if(buf == NULL){return NULL;}

	readlen = 0;
	while(readlen < filelen){
		read = fread(buf+readlen,1,filelen-readlen,fp);
		if(read == 0){
			free(buf);
			return NULL;
		}
		readlen += read;
	}
	fclose(fp);
	buf[filelen] = 0;

	*len = filelen;
	return buf;
}

void doRepl(lClosure *c){
	static char buf[512];
	while(1){
		printf("%sλ%s>%s ",ansiFG[1],ansiFG[12],ansiRS);
		fflush(stdout);
		if(fgets(buf,sizeof(buf),stdin) == NULL){
			printf("Bye!\n");
			return;
		}
		lVal *v = NULL;
		for(lVal *sexpr = lParseSExprCS(buf); sexpr != NULL; sexpr = sexpr->next){
			//printf("Chain:\n");
			//lPrintChain(sexpr);
			v = lEval(c,sexpr);
		}
		lPrintChain(v);
		lClosureGC();
	}
}

lVal *lnfQuit(lClosure *c, lVal *v){
	if(v == NULL){exit(0);}
	lVal *t = lEval(c,v);
	if(t->type != ltInt){exit(0);}
	exit(t->vInt);
	return t;
}

int main(int argc, char *argv[]){
	int eval = 0;
	int repl = 1;
	lInit();
	lClosure *c = lClosureNew(NULL);
	lClosureAddNF(c,"quit",&lnfQuit);
	lClosureAddNF(c,"exit",&lnfQuit);

	for(int i=1;i<argc;i++){
		size_t len;
		char *str = argv[i];
		if(argv[i][0] == '-'){
			if(argv[i][1] == 'e'){
				eval = 1;
				continue;
			}else if(argv[i][1] == '-'){
				repl = 1;
			}
		}
		if(!eval){
			str = loadFile(argv[i],&len);
		}
		lVal *v = NULL;
		for(lVal *sexpr = lParseSExprCS(str); sexpr != NULL; sexpr = sexpr->next){
			//printf("Chain:\n");
			//lPrintChain(sexpr);
			v = lEval(c,sexpr);
		}
		lPrintChain(v);
		lClosureGC();

		if(!eval){
			free(str);
			eval = 0;
		}
		repl = 0;
	}
	if(repl){
		doRepl(c);
	}
	lClosureFree(c);
	return 0;
}
