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

#ifdef __MINGW32__
#include <windows.h>
#include <shlobj.h>
#endif

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

	#if defined (__EMSCRIPTEN__)
	*len = 0;
	(void)filename;
	return NULL;
	#endif

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
	#if defined (__EMSCRIPTEN__)
	(void)filename;
	(void)buf;
	(void)len;
	return;
	#endif

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
	#if defined (__EMSCRIPTEN__)
	(void)name;
	return 0;
	#endif
	DIR *dp = opendir(name);
	if(dp == NULL){return 0;}
	closedir(dp);
	return 1;
}

int isFile(const char *name){
	FILE *fp = fopen(name,"r");
	if(fp == NULL){return 0;}
	fclose(fp);
	return 1;
}

void makeDir(const char *name){
	if(isDir(name)){return;}
	#ifdef __MINGW32__
	mkdir(name);
	#elif defined (__EMSCRIPTEN__)
	(void)name;
	#else
	mkdir(name,0755);
	#endif
}

void makeDirR(const char *name){
	char buf[256];
	strncpy(buf,name,sizeof(buf));
	buf[sizeof(buf)-1] = 0;
	for(int i=0;i<256;i++){
		if(buf[i] != '/'){continue;}
		buf[i] = 0;
		makeDir(buf);
		buf[i] = '/';
	}
	makeDir(buf);
}

void rmDirR(const char *name){
	#if defined (__EMSCRIPTEN__)
	return;
	#endif
	DIR *dp = opendir(name);
	if(dp == NULL){return;}
	struct dirent *de = NULL;
	while((de = readdir(dp)) != NULL){
		char buf[512];
		if(de->d_name[0] == '.'){continue;}
		snprintf(buf,sizeof(buf),"%s/%s",name,de->d_name);
		if(isDir(buf)){
			rmDirR(buf);
			rmdir(buf);
		}else{
			unlink(buf);
		}
	}
	closedir(dp);
	rmdir(name);
}

// Sets current working directory for game files, per default ~/.wolkenwelten/.
// Create directories as needed.
void changeToDataDir(){
	char buf[512];
	#if defined (__EMSCRIPTEN__)
	const char* dir  = "WolkenWelten";
	return;
	#elif defined (__MINGW32__)
	const char* dir  = "WolkenWelten";
	#else
	const char* dir  = ".wolkenwelten";
	#endif
	const char* wwdir = getenv("WOLKENWELTEN_DIR");
	if(wwdir != NULL){
		if(chdir(wwdir) == -1){perror("chdir");}
		return;
	}

	#ifdef __MINGW32__
	char home[512];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, home);
	if(result != S_OK){return;}
	#else
	const char* home = getenv("HOME");
	if(!home){return;}
	#endif

	if(snprintf(buf,sizeof(buf),"%s/%s",home,dir) <= 0){ // snprintf instead of strcpy/strcat
		fprintf(stderr,"Can't create dataDirBuffer, $HOME too long?\n");
		return;
	}
	buf[sizeof(buf)-1]=0;

	makeDirR(buf); // Do not call mkdir directly because of win32 incompat
	if(!isDir(buf)){
		fprintf(stderr,"Can't create/access data dir at '%s'\n",buf);
		return;
	}
	if(chdir(buf) == -1) {
		perror("chdir");
		return;
	}
}

void parseAnsiCommand(int command, int *fgc, int *bgc){
	switch(command){
	case 0:
		*fgc = 15;
		*bgc = 0;
		break;
	case 1:
		*fgc |= 8;
		break;
	case 11:
		*fgc &= 7;
		break;
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
		*fgc = (*fgc & 0x8) | ((command-30) & 0x7);
		break;
	case 39:
		*fgc = (*fgc & 0x8) | 7;
		break;

	case 40:
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
		*bgc = (*bgc & 0x8) | ((command-40) & 0x7);
		break;
	case 49:
		*bgc = (*bgc & 0x8) | 7;
		break;
	}
}

int parseAnsiCode(const char *start, int *fgc, int *bgc){
	if(start[1] != '['){return 1;}
	const char *cur = &start[2];
	const char *end = cur;
	while(*end != 'm'){
		for(;*end && isdigit((u8)*end);end++){}
		if(*end == 0){break;}
		if((*end == 'm') || (*end == ';')){
			const int command = atoi(cur);
			parseAnsiCommand(command,fgc,bgc);
			if(*end++ == 'm'){break;}
			cur = end;
			continue;
		}
		end++;
	}
	const int ret = end - start;
	return ret;
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
