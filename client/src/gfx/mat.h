#pragma once
#include "../../../common/src/common.h"

extern float matMVP[16];

void matFrustum     (float out[16], float left, float right, float bottom, float top, float znear, float zfar);
void matPerspective (float out[16], float fovInDegrees, float aspectRatio, float znear, float zfar);
void matOrtho       (float out[16], float left, float right, float bottom, float top, float znear, float zfar);
void matIdentity    (float out[16]);
void matTranslation (float out[16], float x, float y, float z);
void matScaling     (float out[16], float x, float y, float z);
void matRotX        (float out[16], float rad);
void matRotY        (float out[16], float rad);
void matRotZ        (float out[16], float rad);
void matAdd         (float out[16], const float a[16], const float b[16]);
void matSub         (float out[16], const float a[16], const float b[16]);
void matMul         (float out[16], const float a[16], const float b[16]);
void matMov         (float out[16], const float a[16]);
void matMulRotX     (float mat[16], float rad);
void matMulRotY     (float mat[16], float rad);
void matMulRotZ     (float mat[16], float rad);
void matMulRotXY    (float mat[16], float yaw, float pitch);
void matMulRotYX    (float mat[16], float yaw, float pitch);
void matMulRotYXZ   (float mat[16], float yaw, float pitch, float roll);
void matMulTrans    (float mat[16], float x, float y, float z);
void matMulScale    (float mat[16], float x, float y, float z);
void matPrint       (const float *mat,const char *title);
vec  matMulVec      (const float mat[16], const vec pos);
