#pragma once
#include "../../../common/src/common.h"

#define SHADER_ATTRIDX_POS       0
#define SHADER_ATTRIDX_TEX       1
#define SHADER_ATTRIDX_COLOR     2
#define SHADER_ATTRIDX_SIZE      3
#define SHADER_ATTRIDX_TRANSPOS  4
#define SHADER_ATTRIDX_FADE      5
#define SHADER_ATTRIDX_FLAG      6
#define SHADER_ATTRIDX_PACKED    7

#define SHADER_ATTRMASK_POS      (1 << SHADER_ATTRIDX_POS)
#define SHADER_ATTRMASK_TEX      (1 << SHADER_ATTRIDX_TEX)
#define SHADER_ATTRMASK_COLOR    (1 << SHADER_ATTRIDX_COLOR)
#define SHADER_ATTRMASK_SIZE     (1 << SHADER_ATTRIDX_SIZE)
#define SHADER_ATTRMASK_TRANSPOS (1 << SHADER_ATTRIDX_TRANSPOS)
#define SHADER_ATTRMASK_FADE     (1 << SHADER_ATTRIDX_FADE)
#define SHADER_ATTRMASK_FLAG     (1 << SHADER_ATTRIDX_FLAG)
#define SHADER_ATTRMASK_PACKED   (1 << SHADER_ATTRIDX_PACKED)

void    shaderInit();
void    shaderInitBlockMesh();
void    shaderFree();
void    shaderBind       (shader *s);
void    shaderMatrix     (shader *s, float mvp[16]);
void    shaderAlpha      (shader *s, float alpha);
void    shaderColor      (shader *s, float r, float g, float b, float a);
void    shaderSideTint   (shader *s, const vec sideTint);
void    shaderTransform  (shader *s, float x, float y, float z);
void    shaderSizeMul    (shader *s, float sizeMul);

extern shader *sMesh;
extern shader *sShadow;
extern shader *sBlockMesh;
extern shader *sParticle;
extern shader *sRain;
extern shader *sTextMesh;
extern shader *sCloud;
extern shader *sBoundary;
