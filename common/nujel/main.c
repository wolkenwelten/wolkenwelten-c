#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "assets.h"
#include "../src/common.h"
#include "../src/tmp/assets.h"
#include "../src/nujel/nujel.h"
#include "../src/nujel/casting.h"
#include "../src/nujel/reader.h"
#include "../src/nujel/string.h"

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
	static char str[4096];
	lVal *lastlsym = lValSym("lastl");
	lVal *lastl = lDefineClosureSym(c, lastlsym->vSymbol);
	while(1){
		printf("%sλ%s>%s ",ansiFG[1],ansiFG[12],ansiRS);
		fflush(stdout);
		if(fgets(str,sizeof(str),stdin) == NULL){
			printf("Bye!\n");
			return;
		}
		lVal *v = lEval(c,lWrap(lRead(str)));
		lWriteVal(v);
		lClosureGC();
		lVal *tmp = lValString(str);
		if((tmp != NULL) && (lastl != NULL)){lastl->vList.car = tmp;}
	}
}

lVal *lnfQuit(lClosure *c, lVal *v){
	int ecode = 0;
	if(v != NULL){
		lVal *t = lnfInt(c,lEval(c,v->vList.car));
		if((t != NULL) && (t->type == ltInt)){
			ecode = t->vInt;
		}
	}
	exit(ecode);
	return NULL;
}

lVal *lnfInput(lClosure *c, lVal *v){
	static char buf[512];
	if(v != NULL){
		lVal *t = lnfCat(c,v);
		if((t != NULL) && (t->type == ltString)){
			printf("%s",t->vString->data);
		}
	}
	if(fgets(buf,sizeof(buf),stdin) == NULL){
		return NULL;
	}
	return lValString(buf);
}

lVal *lnfPrint(lClosure *c, lVal *v){
	if(v == NULL){return v;}
	lVal *t = NULL;
	if(v->type == ltPair){
		t = lEval(c,v->vList.car);
	}else{
		t = lEval(c,v);
	}
	lDisplayVal(t);
	return t;
}

void lPrintError(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
}

void addNativeFuncs(lClosure *c){
	lAddNativeFunc(c,"print",  "(...args)","Displays ...args",                            lnfPrint);
	lAddNativeFunc(c,"display","(...args)","Prints ...args",                              lnfPrint);
	lAddNativeFunc(c,"input",  "()",       "Reads in a line of user input and returns it",lnfInput);
	lAddNativeFunc(c,"quit",   "(a)",      "Exits with code a",                           lnfQuit);
	lAddNativeFunc(c,"exit",   "(a)",      "Quits with code a",                           lnfQuit);
}

int main(int argc, char *argv[]){
	int eval = 0;
	int repl = 1;
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	lInit();
	lClosure *c = lClosureNewRoot();
	addNativeFuncs(c);
	lEval(c,lWrap(lRead((const char *)nujel_saolib_nuj_data)));
	lClosureGC();

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
		if(!eval){str = loadFile(argv[i],&len);}
		lVal *v = lEval(c,lWrap(lRead(str)));
		lWriteVal(v);
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
