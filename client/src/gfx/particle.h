#pragma once
#include "../../../common/src/common.h"

extern uint particleCount;
extern uint sparticleCount;

void particleInit();
void newSparticleV(const vec pos,const vec v,const vec vv,float size, float vsize,uint rgba,uint ttl);
void newParticleV(const vec pos,const vec v,const vec vv,float size, float vsize,uint rgba,uint ttl);
void newParticleS(float x,float y,float z, unsigned int nrgba,float power,int nttl);
void newParticle (float x,float y,float z,float vx,float vy,float vz,float vvx,float vvy,float vvz,float size,float vsize,unsigned int nrgba,int nttl);
void particleUpdate();
void particleDraw();
