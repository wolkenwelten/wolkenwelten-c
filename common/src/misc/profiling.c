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

#include "profiling.h"

#include "../misc/misc.h"
#include "../../../client/src/sdl/sdl.h"

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
	u64 worstCase;
} profEntry;

#define PROFILER_MAX_ENTRIES 64
profEntry profEntryList[PROFILER_MAX_ENTRIES];
uint      profEntryCount = 0;

typedef struct {
	u64 total;
	u64 count;
} nprofEntry;

profEntry nprofEntryList[256];
char reportBuf[1<<16];

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
	if(i >= PROFILER_MAX_ENTRIES){return;}
	profEntryList[i].ustart = getUTicks();
}

void profStop(uint i){
	if(i >= PROFILER_MAX_ENTRIES){return;}
	u64 cticks = getUTicks();
	if(profEntryList[i].ustart < cticks){
		const u64 curDuration = cticks - profEntryList[i].ustart;
		profEntryList[i].total += curDuration;
		profEntryList[i].worstCase = MAX(profEntryList[i].worstCase,curDuration);
	}
	profEntryList[i].count++;
}

u64 profGetTotal(uint i){
	if(i >= PROFILER_MAX_ENTRIES){return 0;}
	return profEntryList[i].total;
}

u64 profGetCount(uint i){
	if(i >= PROFILER_MAX_ENTRIES){return 0;}
	return profEntryList[i].count;
}

double profGetMean(uint i){
	if(i >= PROFILER_MAX_ENTRIES){return 0;}
	return (double)profEntryList[i].total / (double)profEntryList[i].count;
}

double profGetWorst(uint i){
	if(i >= PROFILER_MAX_ENTRIES){return 0;}
	return (double)profEntryList[i].worstCase / 1000000.0;
}

double profGetShare(uint i){
	if(i >= PROFILER_MAX_ENTRIES){return 0;}
	u64 totalSum = 0;
	for(uint ii=1;ii < 64;ii++){
		totalSum += profEntryList[ii].total;
	}
	return (double)profEntryList[i].total / (double)totalSum;
}

const char *profGetName(uint i){
	if(i >= PROFILER_MAX_ENTRIES){return "Out of Bounds";}
	return profEntryList[i].funcName;
}

const char *profGetReport(){
	char *buf = reportBuf;
	buf += snprintf(buf,sizeof(reportBuf)-(buf-reportBuf),"%sProfiling Report%s\n",ansiFG[2],ansiRS);
	for(uint i=1;i<64;i++){
		if(profEntryList[i].count == 0){continue;}
		buf += snprintf(buf,sizeof(reportBuf)-(buf-reportBuf),"%5.2f%% %-24s Avg.: %7.4fms Count: %6u WorstCase: %8.4fms\n",profGetShare(i)*100.0,profEntryList[i].funcName, profGetMean(i) / 1000000.0, (uint)profGetCount(i),profGetWorst(i));
	}
	*buf = 0;
	return reportBuf;
}

void profReset(){
	memset(nprofEntryList,0,sizeof(profEntryList));
}
void nprofReset(){
	memset(nprofEntryList,0,sizeof(nprofEntryList));
}

void nprofAddPacket(uint type, uint size){
	nprofEntryList[type&0xFF].count++;
	nprofEntryList[type&0xFF].total+=size;
}

double nprofGetShare(uint type){
	u64 totalSum = 0;
	if(type == 0xFF){return 0.0;}
	for(uint i=1;i < 255;i++){
		totalSum += nprofEntryList[i].total;
	}
	return (double)nprofEntryList[type].total / (double)totalSum;
}

void profileStartupTime(const char *msg){
	static u64 last = 0;
	const u64 cur = getTicks();
	if(last == 0){last = cur;}
	fprintf(stderr, "[%u] - %s\n", (int)(cur - last), msg);
	last = cur;
}
