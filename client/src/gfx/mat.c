#include "mat.h"
#include "../../../common/src/common.h"

#include <stdio.h>
#include <math.h>

void matFrustum(float out[16], float left, float right, float bottom, float top, float znear, float zfar){
	float temp  = 2.f * znear;
	float temp2 = right - left;
	float temp3 = top - bottom;
	float temp4 = zfar - znear;

	out[ 0] =  temp / temp2;
	out[ 1] =  0.0f;
	out[ 2] =  0.0f;
	out[ 3] =  0.0f;

	out[ 4] =  0.0f;
	out[ 5] =  temp / temp3;
	out[ 6] =  0.0f;
	out[ 7] =  0.0f;

	out[ 8] =  ( right + left  ) / temp2;
	out[ 9] =  ( top   + bottom) / temp3;
	out[10] =  (-zfar  - znear ) / temp4;
	out[11] = -1.0f;

	out[12] =  0.0f;
	out[13] =  0.0f;
	out[14] =  (-temp * zfar)  / temp4;
	out[15] =  0.0f;
}

void matPerspective(float out[16], float fovInDegrees, float aspectRatio, float znear, float zfar){
	float ymax = znear * tanf(fovInDegrees * PI / 360.0f);
	float xmax = ymax * aspectRatio;
	matFrustum(out, -xmax, xmax, -ymax, ymax, znear, zfar);
}

void matOrtho(float out[16], float left, float right, float bottom, float top, float znear, float zfar){
	out[ 0] =  2.0f / (right - left);
	out[ 1] =  0.0f;
	out[ 2] =  0.0f;
	out[ 3] =  0.0f;

	out[ 4] =  0.0f;
	out[ 5] =  2.0f / (top - bottom);
	out[ 6] =  0.0f;
	out[ 7] =  0.0f;

	out[ 8] =  0.0f;
	out[ 9] =  0.0f;
	out[10] = -2.0f / (zfar - znear);
	out[11] = -1.0f;

	out[12] = -(right + left  ) / (right - left  );
	out[13] = -(top   + bottom) / (top   - bottom);
	out[14] = -(zfar  + znear ) / (zfar  - znear );
	out[15] =  1.0f;
}

void matIdentity(float out[16]){
	out[ 0] = 1.f;
	out[ 1] = 0.f;
	out[ 2] = 0.f;
	out[ 3] = 0.f;

	out[ 4] = 0.f;
	out[ 5] = 1.f;
	out[ 6] = 0.f;
	out[ 7] = 0.f;

	out[ 8] = 0.f;
	out[ 9] = 0.f;
	out[10] = 1.f;
	out[11] = 0.f;

	out[12] = 0.f;
	out[13] = 0.f;
	out[14] = 0.f;
	out[15] = 1.f;
}

void matTranslation(float out[16],float x, float y, float z){
	matIdentity(out);
	out[12] = x;
	out[13] = y;
	out[14] = z;
}

void matScaling(float out[16],float x, float y, float z){
	matIdentity(out);
	out[ 0] = x;
	out[ 5] = y;
	out[10] = z;
}

void matRotX(float out[16], float rad){
	matIdentity(out);
	out[ 5] =  cosf(rad);
	out[ 6] =  sinf(rad);
	out[ 9] = -sinf(rad);
	out[10] =  cosf(rad);
}

void matRotY(float out[16], float rad){
	matIdentity(out);
	out[ 0] =  cosf(rad);
	out[ 2] = -sinf(rad);
	out[ 8] =  sinf(rad);
	out[10] =  cosf(rad);
}

void matRotZ(float out[16], float rad){
	matIdentity(out);
	out[ 0] =  cosf(rad);
	out[ 1] =  sinf(rad);
	out[ 4] = -sinf(rad);
	out[ 5] =  cosf(rad);
}

void matAdd(float out[16], const float a[16], const float b[16]){
	for(int i=0;i<16;i++){
		out[i] = a[i] + b[i];
	}
}

void matSub(float out[16], const float a[16], const float b[16]){
	for(int i=0;i<16;i++){
		out[i] = a[i] - b[i];
	}
}

void matMov(float out[16], const float a[16]){
	for(int i=0;i<16;i++){
		out[i] = a[i];
	}
}

void matMul(float out[16], const float a[16], const float b[16]){
	float tmp[16];
	tmp[ 0] = (a[ 0] * b[ 0]) + (a[ 1] * b[ 4]) + (a[ 2] * b[ 8]) + (a[ 3] * b[12]);
	tmp[ 1] = (a[ 0] * b[ 1]) + (a[ 1] * b[ 5]) + (a[ 2] * b[ 9]) + (a[ 3] * b[13]);
	tmp[ 2] = (a[ 0] * b[ 2]) + (a[ 1] * b[ 6]) + (a[ 2] * b[10]) + (a[ 3] * b[14]);
	tmp[ 3] = (a[ 0] * b[ 3]) + (a[ 1] * b[ 7]) + (a[ 2] * b[11]) + (a[ 3] * b[15]);
	tmp[ 4] = (a[ 4] * b[ 0]) + (a[ 5] * b[ 4]) + (a[ 6] * b[ 8]) + (a[ 7] * b[12]);
	tmp[ 5] = (a[ 4] * b[ 1]) + (a[ 5] * b[ 5]) + (a[ 6] * b[ 9]) + (a[ 7] * b[13]);
	tmp[ 6] = (a[ 4] * b[ 2]) + (a[ 5] * b[ 6]) + (a[ 6] * b[10]) + (a[ 7] * b[14]);
	tmp[ 7] = (a[ 4] * b[ 3]) + (a[ 5] * b[ 7]) + (a[ 6] * b[11]) + (a[ 7] * b[15]);
	tmp[ 8] = (a[ 8] * b[ 0]) + (a[ 9] * b[ 4]) + (a[10] * b[ 8]) + (a[11] * b[12]);
	tmp[ 9] = (a[ 8] * b[ 1]) + (a[ 9] * b[ 5]) + (a[10] * b[ 9]) + (a[11] * b[13]);
	tmp[10] = (a[ 8] * b[ 2]) + (a[ 9] * b[ 6]) + (a[10] * b[10]) + (a[11] * b[14]);
	tmp[11] = (a[ 8] * b[ 3]) + (a[ 9] * b[ 7]) + (a[10] * b[11]) + (a[11] * b[15]);
	tmp[12] = (a[12] * b[ 0]) + (a[13] * b[ 4]) + (a[14] * b[ 8]) + (a[15] * b[12]);
	tmp[13] = (a[12] * b[ 1]) + (a[13] * b[ 5]) + (a[14] * b[ 9]) + (a[15] * b[13]);
	tmp[14] = (a[12] * b[ 2]) + (a[13] * b[ 6]) + (a[14] * b[10]) + (a[15] * b[14]);
	tmp[15] = (a[12] * b[ 3]) + (a[13] * b[ 7]) + (a[14] * b[11]) + (a[15] * b[15]);
	for(int i=0;i<16;i++){
		out[i] = tmp[i];
	}
}

void matMulRotX(float mat[16], float deg){
	float tmp[16];
	matRotX(tmp,deg*PI/180.f);
	matMul(mat,tmp,mat);
}

void matMulRotY(float mat[16], float deg){
	float tmp[16];
	matRotY(tmp,deg);
	matMul(mat,tmp,mat);
}
void matMulRotZ(float mat[16], float deg){
	float tmp[16];
	matRotZ(tmp,deg);
	matMul(mat,tmp,mat);
}

void matMulRotXY(float mat[16], float yaw, float pitch){
	float tmp[16];
	matRotX(tmp,pitch*PI/180.f);
	matMul(mat,tmp,mat);
	matRotY(tmp,yaw*PI/180.f);
	matMul(mat,tmp,mat);
}
void matMulRotYX(float mat[16], float yaw, float pitch){
	float tmp[16];
	matRotY(tmp,yaw*PI/180.f);
	matMul(mat,tmp,mat);
	matRotX(tmp,pitch*PI/180.f);
	matMul(mat,tmp,mat);
}
void matMulRotYXZ(float mat[16], float yaw, float pitch, float roll){
	float tmp[16];
	matRotY(tmp,yaw*PI/180.f);
	matMul(mat,tmp,mat);
	matRotX(tmp,pitch*PI/180.f);
	matMul(mat,tmp,mat);
	matRotZ(tmp,roll*PI/180.f);
	matMul(mat,tmp,mat);
}

void matMulTrans(float mat[16], float x, float y, float z){
	float tmp[16];
	matTranslation(tmp,x,y,z);
	matMul(mat,tmp,mat);
}

void matMulScale(float mat[16], float x, float y, float z){
	float tmp[16];
	matScaling(tmp,x,y,z);
	matMul(mat,tmp,mat);
}

void matPrint(float *mat,const char *title){
	printf("\n [%s]\n",title);
	for(int x=0;x<4;x++){
		for(int i=x;i<16;i+=4){
			printf(" |%10.6f",mat[i]);
		}
		printf(" |\n");
	}
	printf(" +-----------------------------------------------+\n");
}
