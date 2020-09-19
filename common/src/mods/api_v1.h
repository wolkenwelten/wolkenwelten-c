#pragma once
#include "../common.h"

#include "../../../server/src/tmp/objs.h"
#include "../../../server/src/tmp/sfx.h"

void recipeAdd1I             (u16 nResultID, u16 nResultAmount, u16 nIngredID,  u16 nIngredAmount);
void recipeAdd2I             (u16 nResultID, u16 nResultAmount, u16 nIngredID1, u16 nIngredAmount1, u16 nIngredID2, u16 nIngredAmount2);
void ingredientSubstituteAdd (u16 ingredient, u16 substitute);

void grenadeNew(const character *ent, float pwr);
void beamblast (character *ent, float beamSize, float damageMultiplier, float recoilMultiplier, int hitsLeft, int shots, float inaccuracyInc, float inaccuracyMult);

bool characterHP             (      character *c, int addhp);
int  characterGetHP          (const character *c);
int  characterGetMaxHP       (const character *c);
bool characterDamage         (      character *c, int hp);
void characterAddCooldown    (      character *c, int cooldown);
int  characterGetItemAmount  (const character *c, u16 itemID);
int  characterDecItemAmount  (      character *c, u16 itemID, int amount);
bool characterItemReload     (      character *c, item *i, int cooldown);
void characterStartAnimation (      character *c, int index, int duration);
bool characterTryToShoot     (      character *c, item *i, int cooldown, int bulletcount);

void  itemDiscard  (      item *i);
bool  itemIsEmpty  (const item *i);
int   itemCanStack (const item *i, u16 ID);
int   itemIncStack (      item *i, i16 amount);
int   itemDecStack (      item *i, i16 amount);
int   itemGetAmmo  (const item *i);
int   itemIncAmmo  (      item *i, i16 amount);
int   itemDecAmmo  (      item *i, i16 amount);

void     worldBox        (int x, int y, int z, int w, int h, int d, u8 block);
void     worldBoxSphere  (int x, int y, int z, int r, u8 block);
uint8_t  worldGetB       (int x, int y, int z);
chungus *worldTryChungus (int x, int y, int z);
chungus *worldGetChungus (int x, int y, int z);
bool     worldSetB       (int x, int y, int z, u8 block);
int      checkCollision  (int x, int y, int z);

void sfxPlay(sfx *b, float volume);
void sfxLoop(sfx *b, float volume);
