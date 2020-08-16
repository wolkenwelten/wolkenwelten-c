#pragma once

#include "../common.h"

struct item;
typedef struct item item;
struct mesh;
typedef struct mesh mesh;
struct character;
typedef struct character character;

typedef enum blockCategory {
	NONE,
	DIRT,
	STONE,
	WOOD,
	LEAVES
} blockCategory;

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

void  itemDiscard       (item *i);
bool  itemIsEmpty       (item *i);
float itemGetInaccuracy (item *i);
bool  itemIsSingle      (item *i);
bool  itemCanStack      (item *i, uint16_t ID);
bool  itemIncStack      (item *i, int16_t amount);
bool  itemDecStack      (item *i, int16_t amount);
