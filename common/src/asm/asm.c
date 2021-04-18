#include "asm.h"

void asmDetermineSupport();
int asmRoutineSupport = 0;

#if defined(__arm__) || defined(__aarch64__)
	void      rainPosUpdateNEON();
	void  particlePosUpdateNEON();
	void sparticlePosUpdateNEON();

	void asmDetermineSupport(){
		asmRoutineSupport = 2;
	}

	void particlePosUpdate(){
		if(asmRoutineSupport == 0){asmDetermineSupport();}
		switch(asmRoutineSupport){
		case 0:
		case 1:
		default:
			particlePosUpdatePortable();
			break;
		case 2:
			particlePosUpdateNEON();
			break;
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
		case 2:
			sparticlePosUpdateNEON();
			break;
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
		case 2:
			rainPosUpdateNEON();
			break;
		}
	}
#endif

#ifdef __x86_64__
#if defined(__HAIKU__)
	static void      rainPosUpdateSSE(){}
	static void      rainPosUpdateAVX(){}
	static void  particlePosUpdateSSE(){}
	static void  particlePosUpdateAVX(){}
	static void sparticlePosUpdateSSE(){}
	static void sparticlePosUpdateAVX(){}

	void asmDetermineSupport(){
		asmRoutineSupport = 1;
	}
#else
	void      rainPosUpdateSSE();
	void      rainPosUpdateAVX();
	void  particlePosUpdateSSE();
	void  particlePosUpdateAVX();
	void sparticlePosUpdateSSE();
	void sparticlePosUpdateAVX();

	void asmDetermineSupport(){
		if(__builtin_cpu_supports ("avx")){
			asmRoutineSupport = 3;
		}else if(__builtin_cpu_supports ("sse")){
			asmRoutineSupport = 2;
		}else{
			asmRoutineSupport = 1;
		}
	}
#endif

	void particlePosUpdate(){
		if(asmRoutineSupport == 0){asmDetermineSupport();}
		switch(asmRoutineSupport){
		case 0:
		case 1:
		default:
			particlePosUpdatePortable();
			break;
		case 2:
			particlePosUpdateSSE();
			break;
		case 3:
			particlePosUpdateAVX();
			break;
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
		case 2:
			sparticlePosUpdateSSE();
			break;
		case 3:
			sparticlePosUpdateAVX();
			break;
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
		case 2:
			rainPosUpdateSSE();
			break;
		case 3:
			rainPosUpdateAVX();
			break;
		}
	}
#endif

#ifdef __EMSCRIPTEN__
	void particlePosUpdate(){
		particlePosUpdatePortable();
	}

	void sparticlePosUpdate(){
		sparticlePosUpdatePortable();
	}

	void rainPosUpdate(){
		rainPosUpdatePortable();
	}
#endif
