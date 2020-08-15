#include "misc.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

uint64_t RNGValue = 1;

void seedRNG(uint64_t seed){
	RNGValue = seed;
}

uint64_t getRNGSeed(){
	return RNGValue;
}

uint64_t rngValR(){
	RNGValue = ((RNGValue * 1103515245)) + 12345;
	return ((RNGValue&0xFFFF)<<16) | ((RNGValue>>16)&0xFFFF);
}

float rngValf(){
	return (float)rngValR() / ((float)0xffffffff);
}

uint64_t rngValM(uint64_t max){
	if(max == 0){return 0;}
	return rngValR() % max;
}

int64_t rngValMM(int64_t min,int64_t max){
	return rngValM(max - min) + min;
}

void *loadFile(char *filename,size_t *len){
	FILE *fp;
	size_t filelen,readlen,read;
	uint8_t *buf = NULL;

	fp = fopen(filename,"rb");
	if(fp == NULL){return NULL;}

	fseek(fp,0,SEEK_END);
	filelen = ftell(fp);
	fseek(fp,0,SEEK_SET);

	buf = malloc(filelen);
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

	*len = filelen;
	return buf;
}


const char *getHumanReadableSize(size_t n){
	static char buf[32];
	const char *suffix[] = {"","K","M","G","T"};
	int i;

	for(i=0;i<5;i++){
		if(n<1024){break;}
		n = n >> 10;
	}
	i = snprintf(buf,sizeof(buf),"%llu%s",(long long unsigned int)n,suffix[i]);
	buf[sizeof(buf)-1] = 0;
	return buf;
}

char **splitArgs(const char *cmd,int *rargc){
	static char *argv[32];
	static char buf[1024];
	int mode = 1;
	int argc=0;

	strncpy(buf,cmd,sizeof(buf));
	buf[sizeof(buf)-1] = 0;

	for(char *s = buf;*s!=0;s++){
		if(mode == 2){
			if(*s == '"'){
				*s = 0;
				mode =1;
			}
		} else if(isspace(*s)){
			*s = 0;
			mode = 1;
		}else if(mode == 1){
			if(*s == '"'){
				argv[argc++] = s+1;
				mode = 2;
			}else{
				argv[argc++] = s;
				mode = 0;
			}
			if(argc >= (int)(sizeof(argv)/sizeof(char *))){break;}
		}
	}

	*rargc = argc;
	return argv;
}
