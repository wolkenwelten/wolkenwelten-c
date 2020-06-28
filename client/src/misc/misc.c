#include "misc.h"
#include <stdio.h>
#include <math.h>

unsigned long long int RNGValue = 1;

void seedRNG(unsigned int seed){
	RNGValue = seed;
}

unsigned int getRNGSeed(){
	return RNGValue;
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
