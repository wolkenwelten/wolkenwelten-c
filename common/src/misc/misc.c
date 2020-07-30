#include "misc.h"
#include <stdio.h>
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
