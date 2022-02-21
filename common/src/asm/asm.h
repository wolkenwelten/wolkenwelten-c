#pragma once
#include "../common.h"

extern int asmRoutineSupport;

void asmDetermineSupport();

void  particlePosUpdate();
void sparticlePosUpdate();
void      rainPosUpdate();

void  particlePosUpdatePortable();
void sparticlePosUpdatePortable();
void      rainPosUpdatePortable();


void lightBlurY        (u8 out[48][48][48]);
void lightBlurYPortable(u8 out[48][48][48]);
void lightBlurX        (u8 out[48][48][48]);
void lightBlurXPortable(u8 out[48][48][48]);
void lightBlurZPortable(u8 out[48][48][48]);
