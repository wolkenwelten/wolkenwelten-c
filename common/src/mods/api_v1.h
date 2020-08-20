#pragma once

#include "../common.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

extern mesh *meshPear;
extern mesh *meshHook;
extern mesh *meshGrenade;
extern mesh *meshBomb;
extern mesh *meshAxe;
extern mesh *meshPickaxe;
extern mesh *meshMasterblaster;;

void recipeAdd1I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID, unsigned char nIngredAmount);
void recipeAdd2I(unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID1, unsigned char nIngredAmount1, unsigned short nIngredID2, unsigned char nIngredAmount2);
void ingredientSubstituteAdd(unsigned short ingredient, unsigned short substitute);

void grenadeNew(character *ent, float pwr);
void beamblast (character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult);

bool characterHP           (character *c, int addhp);
int  characterGetHP        (character *c);
int  characterGetMaxHP     (character *c);
bool characterDamage       (character *c, int hp);
void characterAddCooldown  (character *c, int cooldown);
int  characterGetItemAmount(character *c, uint16_t itemID);
int  characterDecItemAmount(character *c, uint16_t itemID, int amount);

void  itemDiscard  (item *i);
bool  itemIsEmpty  (item *i);
int   itemCanStack (item *i, uint16_t ID);
int   itemIncStack (item *i, int16_t amount);
int   itemDecStack (item *i, int16_t amount);
int   itemGetAmmo  (item *i);
int   itemIncAmmo  (item *i, int16_t amount);
int   itemDecAmmo  (item *i, int16_t amount);

void worldBox       (int x, int y, int z, int w, int h, int d, uint8_t block);
void worldBoxSphere (int x, int y, int z, int r, uint8_t block);
uint8_t worldGetB   (int x, int y, int z);
bool worldSetB      (int x, int y, int z, uint8_t block);
int checkCollision  (int x, int y, int z);


extern sfx *sfxFalling;
extern sfx *sfxHoho;
extern sfx *sfxHoo;
extern sfx *sfxImpact;
extern sfx *sfxPhaser;
extern sfx *sfxBomb;
extern sfx *sfxTock;
extern sfx *sfxPock;
extern sfx *sfxStomp;
extern sfx *sfxStep;
extern sfx *sfxUngh;
extern sfx *sfxYahoo;
extern sfx *sfxHookFire;
extern sfx *sfxHookHit;
extern sfx *sfxHookReturned;
extern sfx *atmosfxHookRope;
extern sfx *atmosfxWind;

void sfxPlay(sfx *b, float volume);
void sfxLoop(sfx *b, float volume);
