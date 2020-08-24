#pragma once

#include "../common.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#include "../../../server/src/tmp/objs.h"
#include "../../../server/src/tmp/sfx.h"


void recipeAdd1I            (unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID, unsigned char nIngredAmount);
void recipeAdd2I            (unsigned short nResultID, unsigned char nResultAmount, unsigned short nIngredID1, unsigned char nIngredAmount1, unsigned short nIngredID2, unsigned char nIngredAmount2);
void ingredientSubstituteAdd(unsigned short ingredient, unsigned short substitute);

void grenadeNew(character *ent, float pwr);
void beamblast (character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult);

bool characterHP             (character *c, int addhp);
int  characterGetHP          (character *c);
int  characterGetMaxHP       (character *c);
bool characterDamage         (character *c, int hp);
void characterAddCooldown    (character *c, int cooldown);
int  characterGetItemAmount  (character *c, uint16_t itemID);
int  characterDecItemAmount  (character *c, uint16_t itemID, int amount);
bool characterItemReload     (character *c, item *i, int cooldown);
void characterStartAnimation (character *c, int index, int duration);
bool characterTryToShoot     (character *c, item *i, int cooldown, int bulletcount);

void  itemDiscard  (item *i);
bool  itemIsEmpty  (item *i);
int   itemCanStack (item *i, uint16_t ID);
int   itemIncStack (item *i, int16_t amount);
int   itemDecStack (item *i, int16_t amount);
int   itemGetAmmo  (item *i);
int   itemIncAmmo  (item *i, int16_t amount);
int   itemDecAmmo  (item *i, int16_t amount);

void    worldBox        (int x, int y, int z, int w, int h, int d, uint8_t block);
void    worldBoxSphere  (int x, int y, int z, int r, uint8_t block);
uint8_t worldGetB       (int x, int y, int z);
bool    worldSetB       (int x, int y, int z, uint8_t block);
int     checkCollision  (int x, int y, int z);

void sfxPlay(sfx *b, float volume);
void sfxLoop(sfx *b, float volume);
