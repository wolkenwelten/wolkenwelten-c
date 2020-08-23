/* This file exists so we do not get undefined references when we link in code
 * meant for the client side.
 */

#include <stdio.h>
#include <stdint.h>

struct item;
typedef struct item item;
struct mesh;
typedef struct mesh mesh;
struct character;
typedef struct character character;
struct sfx;
typedef struct sfx sfx;

sfx *sfxFalling;
sfx *sfxHoho;
sfx *sfxHoo;
sfx *sfxImpact;
sfx *sfxPhaser;
sfx *sfxBomb;
sfx *sfxTock;
sfx *sfxPock;
sfx *sfxStomp;
sfx *sfxStep;
sfx *sfxUngh;
sfx *sfxYahoo;
sfx *sfxHookFire;
sfx *sfxHookHit;
sfx *sfxHookReturned;
sfx *atmosfxHookRope;
sfx *atmosfxWind;

mesh *meshPear;
mesh *meshHook;
mesh *meshGrenade;
mesh *meshBomb;
mesh *meshAxe;
mesh *meshPickaxe;
mesh *meshMasterblaster;
mesh *meshBlaster;
mesh *meshCrystalbullet;
mesh *meshAssaultblaster;
mesh *meshShotgunblaster;

void recipeAdd1I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID, unsigned char nIngredAmount){
	(void)nResultID;
	(void)nResultAmount;
	(void)nIngredID;
	(void)nIngredAmount;

	fprintf(stderr,"recipeAdd1I got called on the serverside\n");
}
void recipeAdd2I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID1, unsigned char nIngredAmount1, unsigned short nIngredID2, unsigned char nIngredAmount2){
	(void)nResultID;
	(void)nResultAmount;
	(void)nIngredID1;
	(void)nIngredAmount1;
	(void)nIngredID2;
	(void)nIngredAmount2;

	fprintf(stderr,"recipeAdd2I got called on the serverside\n");
}

void ingredientSubstituteAdd(unsigned short ingredient, unsigned short substitute){
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
void blockTypeSetTex(uint8_t b, int side, uint32_t tex){
	(void)b;
	(void)side;
	(void)tex;
}
