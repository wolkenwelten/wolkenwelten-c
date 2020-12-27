#include "time.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

u32 gameTime;

uint gtimeGetTime(){
	return gameTime;
}

uint gtimeGetTimeOfDay(){
	return gameTime & 0xFFFFF;
}

void gtimeSetTimeOfDay(uint newTime){
	gameTime = (gameTime & ~0xFFFFF) | (newTime & 0xFFFFF);
}

void gtimeSetTimeOfDayHR (uint newHours, uint newMinutes){
	const uint hour =   (1<<20)/24;
	const uint minute = (1<<20)/(24*60);
	gtimeSetTimeOfDay(newHours*hour + ((newMinutes%60)*minute));
}

void gtimeSetTimeOfDayHRS(const char *newTime){
	int startHour    = -1;
	int startMinutes = -1;

	for(int i=0;i<32;i++){
		u8 c = newTime[i];
		if(c == 0){break;}
		if(isdigit(c) && (startHour < 0)){
			startHour = i;
			continue;
		}
		if((c == ':') && (startMinutes < 0)){
			startMinutes = i+1;
			continue;
		}
		if(startMinutes > 0){
			if(isdigit(c)){break;}
			startMinutes = i;
		}
	}
	if(startMinutes < 0){return;}
	int hours = atoi(&newTime[startHour]);
	int minutes = atoi(&newTime[startMinutes]);
	gtimeSetTimeOfDayHR(hours,minutes);
}

void gtimeSetTime(uint newTime){
	gameTime = newTime;
}

void gtimeUpdate(){
	gameTime++;
}

const char *gtimeGetTimeOfDayHRS(uint timeCur){
	static char buf[32];
	const uint timeHours = timeCur / ((1<<20)/24);
	const uint timeMinutes = (timeCur / ((1<<20)/(24*60)))%60;
	snprintf(buf,sizeof(buf),"%2.u:%2.u",timeHours,timeMinutes);
	buf[sizeof(buf)-1]=0;
	return buf;
}

uint gtimeGetTimeCat(){
	return (gameTime >> 18) & 0x3;
}

float gtimeGetBrightness(uint time){
	const float t = .5f + (time / (float)(1<<20));
	const float tv = t*(PI*2);
	return MINMAX(0.3f,1.0f,cosf(tv) * 1.5f);
}
