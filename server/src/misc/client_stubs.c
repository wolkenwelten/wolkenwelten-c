/* This file exists so we do not get undefined references when we link in code
 * meant for the client side.
 */
#include "../../../common/src/common.h"

#include <stdio.h>

uint glParticles;
uint   particles;
uint   particleCount;
uint glSparticles;
uint   sparticles;
uint   sparticleCount;

__attribute__((aligned(32))) const float sparticleVV[4][4] = {
	{  0.000001f,0.00004f, 0.000004f, 0.f},
	{ -0.000004f,0.00004f, 0.000001f, 0.f},
	{ -0.000001f,0.00004f,-0.000004f, 0.f},
	{  0.000004f,0.00004f,-0.000001f, 0.f},
};
void particlePosUpdatePortable(){}
void sparticlePosUpdatePortable(){}

void fxRainDrop(const vec pos){
	(void)pos;
}

void fxBlockBreak(const vec pos, u8 b, u8 cause){
	(void)pos;
	(void)b;
	(void)cause;
}

void recipeNew1(const item result, const item ingred1){
	(void)result;
	(void)ingred1;
	fprintf(stderr,"recipeNew1 got called on the serverside\n");
}

void recipeNew2(const item result, const item ingred1, const item ingred2){
	(void)result;
	(void)ingred1;
	(void)ingred2;
	fprintf(stderr,"recipeNew2 got called on the serverside\n");
}

void recipeNew3(const item result, const item ingred1, const item ingred2, const item ingred3){
	(void)result;
	(void)ingred1;
	(void)ingred2;
	(void)ingred3;
	fprintf(stderr,"recipeNew3 got called on the serverside\n");
}

void recipeNew4(const item result, const item ingred1, const item ingred2, const item ingred3, const item ingred4){
	(void)result;
	(void)ingred1;
	(void)ingred2;
	(void)ingred3;
	(void)ingred4;
	fprintf(stderr,"recipeNew4 got called on the serverside\n");
}

void ingredientSubstituteAdd(u16 ingredient, u16 substitute){
	(void)ingredient;
	(void)substitute;

	fprintf(stderr,"ingredientSubstituteAdd got called on the serverside\n");
}

void beamblast(character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult){
	(void)ent;
	(void)beamSize;
	(void)damageMultiplier;
	(void)recoilMultiplier;
	(void)hitsLeft;
	(void)shots;
	(void)inaccuracyInc;
	(void)inaccuracyMult;

	fprintf(stderr,"beamblast got called on the serverside\n");
}

void sfxPlay(sfx *b, float volume){
	(void)b;
	(void)volume;
}
void sfxLoop(sfx *b, float volume){
	(void)b;
	(void)volume;
}

void blockTypeGenMeshes(){}
void blockTypeSetTex(u8 b, int side, u32 tex){
	(void)b;
	(void)side;
	(void)tex;
}
