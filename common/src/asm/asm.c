#include "asm.h"

int asmRoutineSupport = 0;

#if defined(__arm__) || defined(__aarch64__)
	#define ASM_USE_NEON
#endif

#if defined(__x86_64__)
	#define ASM_USE_SSE
	#define ASM_USE_AVX
#endif

/* MacOS has issues because it doesn't include GAS, and Haiku doesn't work with ASMfor now */
#if defined(__APPLE__) || defined(__HAIKU__)
	#undef ASM_USE_SSE
	#undef ASM_USE_AVX
#endif

void asmDetermineSupport(){
	asmRoutineSupport = 1;
#ifdef ASM_USE_SSE
	if(__builtin_cpu_supports ("sse")){asmRoutineSupport = 2;}
#endif
#ifdef ASM_USE_AVX
	if(__builtin_cpu_supports ("avx")){asmRoutineSupport = 3;}
#endif
#ifdef ASM_USE_NEON
	/* Current ARM chips without Neon will have trouble running the game anyways */
	asmRoutineSupport = 4;
#endif
}

#ifdef ASM_USE_NEON
	void      rainPosUpdateNEON();
	void  particlePosUpdateNEON();
	void sparticlePosUpdateNEON();
#endif

#ifdef ASM_USE_SSE
	void      rainPosUpdateSSE();
	void  particlePosUpdateSSE();
	void sparticlePosUpdateSSE();
#endif

#ifdef ASM_USE_AVX
	void      rainPosUpdateAVX();
	void  particlePosUpdateAVX();
	void sparticlePosUpdateAVX();
#endif

void particlePosUpdate(){
	if(asmRoutineSupport == 0){asmDetermineSupport();}
	switch(asmRoutineSupport){
	case 0:
	case 1:
	default:
		particlePosUpdatePortable();
		break;
#ifdef ASM_USE_SSE
	case 2:
		particlePosUpdateSSE();
		break;
#endif
#ifdef ASM_USE_AVX
	case 3:
		particlePosUpdateAVX();
		break;
#endif
#ifdef ASM_USE_NEON
	case 4:
		particlePosUpdateNEON();
		break;
#endif
	}
}

void sparticlePosUpdate(){
	if(asmRoutineSupport == 0){asmDetermineSupport();}
	switch(asmRoutineSupport){
	case 0:
	case 1:
	default:
		sparticlePosUpdatePortable();
		break;
#ifdef ASM_USE_SSE
	case 2:
		sparticlePosUpdateSSE();
		break;
#endif
#ifdef ASM_USE_AVX
	case 3:
		sparticlePosUpdateAVX();
		break;
#endif
#ifdef ASM_USE_NEON
	case 4:
		sparticlePosUpdateNEON();
		break;
#endif
	}
}

void rainPosUpdate(){
	if(asmRoutineSupport == 0){asmDetermineSupport();}
	switch(asmRoutineSupport){
	case 0:
	case 1:
	default:
		rainPosUpdatePortable();
		break;
#ifdef ASM_USE_SSE
	case 2:
		rainPosUpdateSSE();
		break;
#endif
#ifdef ASM_USE_AVX
	case 3:
		rainPosUpdateAVX();
		break;
#endif
#ifdef ASM_USE_NEON
	case 4:
		rainPosUpdateNEON();
		break;
#endif
	}
}
