#include "profiling.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

typedef struct profEntry {
    const char *funcName;
    u64 total;
    u64 ustart;
    u64 count;
} profEntry;

#define PROFILER_MAX_ENTRIES 64
profEntry profEntryList[PROFILER_MAX_ENTRIES];
uint      profEntryCount = 0;

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
	static char retBuf[4096];
	char *buf = retBuf;
	buf += snprintf(buf,sizeof(retBuf)-(buf-retBuf),"%sProfiling Report%s\n",ansiFG[2],ansiRS);
	for(uint i=1;i<64;i++){
		if(profEntryList[i].count == 0){continue;}
		buf += snprintf(buf,sizeof(retBuf)-(buf-retBuf),"[%02u] %5.2f%% %-24s Avg.: %6.4fms\n",i,profGetShare(i)*100.0,profEntryList[i].funcName, profGetMean(i) / 1000000.0 );
	}
	*buf = 0;
	return retBuf;
}
