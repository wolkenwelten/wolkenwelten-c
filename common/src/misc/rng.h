#pragma once
#include "../stdint.h"

extern u64 RNGValue;

static inline void seedRNG(u64 seed){
	RNGValue = seed;
}

static inline u64 getRNGSeed(){
	return RNGValue;
}

static inline u64 rngValR(){
	RNGValue = ((RNGValue * 1103515245)) + 12345;
	return RNGValue;
}

static inline float rngValf(){
	return (float)rngValR() / ((float)0xffffffff);
}

static inline u64 rngValA(u64 mask){
	return rngValR() & mask;
}

static inline u64 rngValM(u64 max){
	return rngValR() % max;
}

static inline i64 rngValMM(i64 min,i64 max){
	return rngValM(max - min) + min;
}
