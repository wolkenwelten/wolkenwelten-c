#pragma once
#include "../../../common/src/common.h"

extern uint particleCount;
extern uint sparticleCount;

void particleInit();
void newSparticleV(const vec pos,const vec v, float size, float vsize, u32 rgba, uint ttl);
void newParticleV (const vec pos,const vec v, float size, float vsize, u32 rgba, uint ttl);
void newParticleS (float x,float y,float z, u32 nrgba, float power, uint nttl);
void newParticle  (float x,float y,float z, float vx,float vy,float vz,float size,float vsize,u32 nrgba, uint nttl);
void particleUpdate();
void particleDraw();
