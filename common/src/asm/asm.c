#include "asm.h"
#include "../misc/profiling.h"

int asmRoutineSupport = 0;

#if defined(__arm__) || defined(__aarch64__)
	#define ASM_TRY_NEON
#endif

#if defined(__x86_64__)
	#define ASM_TRY_SSE
	#define ASM_TRY_AVX
#endif

/* Haiku doesn't work with ASM for now */
#if defined(__HAIKU__)
	#undef ASM_TRY_SSE
	#undef ASM_TRY_AVX
#endif

void asmDetermineSupport(){
	asmRoutineSupport = 1;
#ifdef ASM_TRY_SSE
	if(__builtin_cpu_supports ("sse")){asmRoutineSupport = 2;}
#endif
#ifdef ASM_TRY_AVX
	if(__builtin_cpu_supports ("avx")){asmRoutineSupport = 3;}
#endif
#ifdef ASM_TRY_NEON
	/* Current ARM chips without Neon will have trouble running the game anyways */
	asmRoutineSupport = 4;
#endif
}

#ifdef ASM_TRY_NEON
	void      rainPosUpdateNEON();
	void  particlePosUpdateNEON();
	void sparticlePosUpdateNEON();
#endif

#ifdef ASM_TRY_SSE
	void      rainPosUpdateSSE();
	void  particlePosUpdateSSE();
	void sparticlePosUpdateSSE();
	void         lightBlurXSSE(u8 out[48][48][48]);
	void         lightBlurYSSE(u8 out[48][48][48]);
#endif

#ifdef ASM_TRY_AVX
	void      rainPosUpdateAVX();
	void  particlePosUpdateAVX();
	void sparticlePosUpdateAVX();
#endif

void particlePosUpdate(){
	switch(asmRoutineSupport){
	case 0:
	case 1:
	default:
		particlePosUpdatePortable();
		break;
#ifdef ASM_TRY_SSE
	case 2:
		particlePosUpdateSSE();
		break;
#endif
#ifdef ASM_TRY_AVX
	case 3:
		particlePosUpdateAVX();
		break;
#endif
#ifdef ASM_TRY_NEON
	case 4:
		particlePosUpdateNEON();
		break;
#endif
	}
}

void sparticlePosUpdate(){
	switch(asmRoutineSupport){
	case 0:
	case 1:
	default:
		sparticlePosUpdatePortable();
		break;
#ifdef ASM_TRY_SSE
	case 2:
		sparticlePosUpdateSSE();
		break;
#endif
#ifdef ASM_TRY_AVX
	case 3:
		sparticlePosUpdateAVX();
		break;
#endif
#ifdef ASM_TRY_NEON
	case 4:
		sparticlePosUpdateNEON();
		break;
#endif
	}
}

void rainPosUpdate(){
	switch(asmRoutineSupport){
	case 0:
	case 1:
	default:
		rainPosUpdatePortable();
		break;
#ifdef ASM_TRY_SSE
	case 2:
		rainPosUpdateSSE();
		break;
#endif
#ifdef ASM_TRY_AVX
	case 3:
		rainPosUpdateAVX();
		break;
#endif
#ifdef ASM_TRY_NEON
	case 4:
		rainPosUpdateNEON();
		break;
#endif
	}
}

void lightBlurX(u8 out[48][48][48]){
	PROFILE_START();
	switch(asmRoutineSupport){
	case 0:
	case 1:
	default:
		lightBlurXPortable(out);
		break;
#ifdef ASM_TRY_SSE
	case 2:
	case 3:
		lightBlurXSSE(out);
		break;
#endif
	}
	PROFILE_STOP();
}

void lightBlurY(u8 out[48][48][48]){
	PROFILE_START();
	switch(asmRoutineSupport){
	case 0:
	case 1:
	default:
		lightBlurYPortable(out);
		break;
#ifdef ASM_TRY_SSE
	case 2:
	case 3:
		lightBlurYSSE(out);
		break;
#endif
	}
	PROFILE_STOP();
}

void lightBlurZ(u8 out[48][48][48]){
	PROFILE_START();
	switch(asmRoutineSupport){
	case 0:
	case 1:
	default:
		lightBlurZPortable(out);
		break;
	}
	PROFILE_STOP();
}
