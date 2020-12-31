#pragma once
#include "../../../common/src/common.h"

void    shaderInit();
void    shaderFree();
shader *shaderNew        (const char *vss,const char *fss,unsigned int attrMask);
void    shaderBind       (shader *s);
void    shaderMatrix     (shader *s, float mvp[16]);
void    shaderAlpha      (shader *s, float alpha);
void    shaderBrightness (shader *s, float v);
void    shaderTransform  (shader *s, float x,float y,float z);
void    shaderSizeMul    (shader *s, float sizeMul);

extern shader *sMesh;
extern shader *sShadow;
extern shader *sBlockMesh;
extern shader *sParticle;
extern shader *sTextMesh;
extern shader *sCloud;
