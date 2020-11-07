#include "misc.h"

#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

u64 RNGValue = 1;

void seedRNG(u64 seed){
	RNGValue = seed;
}

u64 getRNGSeed(){
	return RNGValue;
}

u64 rngValR(){
	RNGValue = ((RNGValue * 1103515245)) + 12345;
	return ((RNGValue&0xFFFF)<<16) | ((RNGValue>>16)&0xFFFF);
}

float rngValf(){
	return (float)rngValR() / ((float)0xffffffff);
}

u64 rngValM(u64 max){
	if(max == 0){return 0;}
	return rngValR() % max;
}

i64 rngValMM(i64 min,i64 max){
	return rngValM(max - min) + min;
}

float animationInterpolation(int left, int max , float midPoint){
	if(max  == 0){return 0.f;}
	if(left <= 0){return 0.f;}
	float ret = 1.f - ((float)left / (float)max);
	if(ret > midPoint){
		return 1.f - ((ret - midPoint)/(1.f - midPoint));
	}else{
		return ret / midPoint;
	}
}

float animationInterpolationSustain(int left, int max , float startPoint, float stopPoint){
	if(max  == 0){return 0.f;}
	if(left <= 0){return 0.f;}
	float ret = 1.f - ((float)left / (float)max);
	if(ret > stopPoint){
		return 1.f - ((ret - stopPoint)/(1.f - stopPoint));
	}else if(ret < startPoint){
		return ret / startPoint;
	}else{
		return 1.f;
	}
}

void *loadFile(const char *filename,size_t *len){
	FILE *fp;
	size_t filelen,readlen,read;
	u8 *buf = NULL;

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

void saveFile(const char *filename,const void *buf, size_t len){
	FILE *fp;
	size_t written,wlen = 0;

	fp = fopen(filename,"wb");
	if(fp == NULL){return;}

	while(wlen < len){
		written = fwrite(buf+wlen,1,len-wlen,fp);
		if(written == 0){return;}
		wlen += written;
	}
	fclose(fp);
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

int isDir(const char *name){
	DIR *dp = opendir(name);
	if(dp == NULL){
		return 0;
	}
	closedir(dp);
	return 1;
}

int isFile(const char *name){
	FILE *fp = fopen(name,"r");
	if(fp == NULL){
		return 0;
	}
	fclose(fp);
	return 1;
}

void makeDir(const char *name){
	#ifdef __MINGW32__
	mkdir(name);
	#elif __EMSCRIPTEN__
	(void)name;
	#else
	mkdir(name,0755);
	#endif
}

void rmDirR(const char *name){
	DIR *dp = opendir(name);
	if(dp == NULL){return;}
	struct dirent *de = NULL;
	while((de = readdir(dp)) != NULL){
		if(de->d_name[0] == '.'){continue;}
		char *buf = malloc(512);
		snprintf(buf,512,"%s/%s",name,de->d_name);
		buf[511]=0;
		if(isDir(buf)){
			rmDirR(buf);
			rmdir(buf);
		}else{
			unlink(buf);
		}
		free(buf);
	}
	closedir(dp);
	rmdir(name);
}

int inWorld(int x, int y, int z){
	if((x < 0) || (y < 0) || (z < 0))               {return 0;}
	if((x >= 65536) || (y >= 32768) || (z >= 65536)){return 0;}
	return 1;
}
