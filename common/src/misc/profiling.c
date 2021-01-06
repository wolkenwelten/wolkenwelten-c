#include "profiling.h"

#include "../misc/misc.h"
#include "../network/messages.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

typedef struct {
    const char *funcName;
    u64 total;
    u64 ustart;
    u64 count;
} profEntry;

#define PROFILER_MAX_ENTRIES 64
profEntry profEntryList[PROFILER_MAX_ENTRIES];
uint      profEntryCount = 0;


typedef struct {
    u64 total;
    u64 count;
} nprofEntry;

profEntry nprofEntryList[256];

static char reportBuf[4096];

uint profGetIndex(const char *funcName){
	const uint index = profEntryCount++;
	if(profEntryCount >= PROFILER_MAX_ENTRIES){
		fprintf(stderr,"ERROR: profEntryList overflow\n");
		exit(1);
	}
	profEntryList[index].funcName = funcName;
	return index;
}

u64 getUTicks(){
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC,&tv);
	return tv.tv_nsec + (tv.tv_sec * 1000000000);
}

void profStart(uint i){
	profEntryList[i].ustart = getUTicks();
}

void profStop(uint i){
	profEntryList[i].total += getUTicks() - profEntryList[i].ustart;
	profEntryList[i].count++;
}

u64 profGetTotal(uint i){
	return profEntryList[i].total;
}

double profGetMean(uint i){
	return (double)profEntryList[i].total / (double)profEntryList[i].count;
}

double profGetShare(uint i){
	u64 totalSum = 0;
	for(uint ii=1;ii < 64;ii++){
		totalSum += profEntryList[ii].total;
	}
	return (double)profEntryList[i].total / (double)totalSum;
}

const char *profGetName(uint i){
	return profEntryList[i].funcName;
}

const char *profGetReport(){
	char *buf = reportBuf;
	buf += snprintf(buf,sizeof(reportBuf)-(buf-reportBuf),"%sProfiling Report%s\n",ansiFG[2],ansiRS);
	for(uint i=1;i<64;i++){
		if(profEntryList[i].count == 0){continue;}
		buf += snprintf(buf,sizeof(reportBuf)-(buf-reportBuf),"%5.2f%% %-24s Avg.: %6.4fms\n",profGetShare(i)*100.0,profEntryList[i].funcName, profGetMean(i) / 1000000.0 );
	}
	*buf = 0;
	return reportBuf;
}

void profReset(){
	for(uint i=1;i<64;i++){
		profEntryList[i].count = 0;
		profEntryList[i].total = 0;
	}
}

void nprofAddPacket(uint type, uint size){
	nprofEntryList[type].count++;
	nprofEntryList[type].total+=size;
}

double nprofGetShare(uint type){
	u64 totalSum = 0;
	if(type == 0xFF){return 0.0;}
	for(uint i=1;i < 255;i++){
		totalSum += nprofEntryList[i].total;
	}
	return (double)nprofEntryList[type].total / (double)totalSum;
}

const char *nprofGetReport(){
	char *buf = reportBuf;
	buf += snprintf(buf,sizeof(reportBuf)-(buf-reportBuf),"%sNetwork Profiling Report%s\n",ansiFG[4],ansiRS);
	for(uint i=1;i<256;i++){
		if(nprofEntryList[i].count == 0){continue;}
		int cc = 8;
		if(nprofEntryList[i].total > (1024*1024*1024)){
			cc = 1;
		}else if(nprofEntryList[i].total > (1024*1024)){
			cc = 3;
		}else if(nprofEntryList[i].total > 1024){
			cc = 15;
		}
		buf += snprintf(buf,sizeof(reportBuf)-(buf-reportBuf),"[%02X] %5.2f%% %-24s Total: %s%s%s\n",i,nprofGetShare(i)*100.0,networkGetMessageName(i), ansiFG[cc],getHumanReadableSize(nprofEntryList[i].total),ansiRS);
	}
	*buf = 0;
	return reportBuf;
}

void nprofReset(){
	memset(nprofEntryList,0,sizeof(nprofEntryList));
}
