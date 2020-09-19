/* This file exists so we do not get undefined references when we link in code
 * meant for the client side.
 */
#include "../../../common/src/common.h"

#include <stdio.h>

void recipeAdd1I(u16 nResultID, u16 nResultAmount, u16 nIngredID, u16 nIngredAmount){
	(void)nResultID;
	(void)nResultAmount;
	(void)nIngredID;
	(void)nIngredAmount;

	fprintf(stderr,"recipeAdd1I got called on the serverside\n");
}
void recipeAdd2I(u16 nResultID, u16 nResultAmount, u16 nIngredID1, u16 nIngredAmount1, u16 nIngredID2, u16 nIngredAmount2){
	(void)nResultID;
	(void)nResultAmount;
	(void)nIngredID1;
	(void)nIngredAmount1;
	(void)nIngredID2;
	(void)nIngredAmount2;

	fprintf(stderr,"recipeAdd2I got called on the serverside\n");
}

void ingredientSubstituteAdd(u16 ingredient, u16 substitute){
	(void)ingredient;
	(void)substitute;

	fprintf(stderr,"ingredientSubstituteAdd got called on the serverside\n");
}

void grenadeNew(character *ent, float pwr){
	(void)ent;
	(void)pwr;

	fprintf(stderr,"grenadeNew got called on the serverside\n");
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
