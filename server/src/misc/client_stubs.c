/* This file exists so we do not get undefined references when we link in code
 * meant for the client side.
 */
#include "../../../common/src/common.h"

#include <stdio.h>

void fxBlockBreak(const vec pos, uchar b){
	(void)pos;
	(void)b;
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
