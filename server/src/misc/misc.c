#include "misc.h"

#include <math.h>
#include <stdio.h>
#include <sys/time.h>

unsigned long long int RNGValue = 1;

void seedRNG(unsigned int seed){
	RNGValue = seed;
}

unsigned int getRNGSeed(){
	return RNGValue;
}

unsigned int getTicks(){
	return 0;
}

float rngValf(){
	RNGValue = ((RNGValue * 1103515245)) + 12345;
	return (float)(((RNGValue&0xFFFF)<<16) | ((RNGValue>>16)&0xFFFF)) / ((float)0xffffffff);
}

unsigned int rngValR(){
	RNGValue = ((RNGValue * 1103515245)) + 12345;
	return ((RNGValue&0xFFFF)<<16) | ((RNGValue>>16)&0xFFFF);
}
unsigned int rngValM(unsigned int max){
	if(max == 0){return 0;}
	return rngValR() % max;
}
int rngValMM(int min,int max){
	return rngValM(max - min) + min;
}

uint64_t getMillis(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
}
