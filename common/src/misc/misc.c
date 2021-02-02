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

#include "misc.h"

#include "sha1.h"

#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

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
		} else if(isspace((u8)*s)){
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

int parseAnsiCode(const char *str, int *fgc, int *bgc){
	int off = 0;
	for(int i=0;i<7;i++){
		if(str[i] == 0){return i;}
	}
	if(str[1] != '['){return 1;}
	if(str[3] != ';'){
		if(str[2] != '0'){return 1;}
		if(str[3] != 'm'){return 1;}
		*fgc = 15;
		return 4;
	}
	if(str[6] != 'm'){return 1;}
	if(str[2] == '1'){
		off = 8;
	}else if(str[2] != '0'){
		return 1;
	}
	if(str[4] != '3'){return 1;}
	if((str[5] < '0') || (str[5] > '8')){return 1;}
	*fgc = off + str[5] - '0';
	*bgc = -1;
	return 7;
}

u64 SHA1Simple(const void *data, uint len){
	SHA1_CTX ctx;
	u8 d[20];
	SHA1Init(&ctx);
	SHA1Update(&ctx,(u8 *)data,len);
	SHA1Final(d,&ctx);
	u64 ret = d[0] | ((u64)d[1] << 8) | ((u64)d[2] << 8) | ((u64)d[3] << 8) | ((u64)d[4] << 8) | ((u64)d[5] << 8) | ((u64)d[6] << 8) | ((u64)d[7] << 8);
	return ret;
}

void strRemove(char *buf, int size, int start, int end){
	const int off = end-start;
	int blen = strnlen(buf,size);
	for(int i=start;i<blen-off;i++){
		buf[i] = buf[i+off];
	}
	buf[blen-off] = 0;
}
void strInsert(char *buf, int size, int start,const char *snippet){
	int off = strlen(snippet);
	int blen = strnlen(buf,size);
	if((off+blen) > size){return;}
	for(int i=blen+off-1;i>=start+off;i--){
		buf[i] = buf[i-off];
	}
	for(int i=start;i<start+off;i++){
		buf[i] = *snippet++;
	}
	buf[blen+off] = 0;
}
