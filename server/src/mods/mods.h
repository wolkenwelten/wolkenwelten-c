#pragma once
#include "../game/blockType.h"
#include "../game/item.h"
#include "../game/character.h"

struct mesh;
typedef struct mesh mesh;

int   blockDamageDispatch     (item *cItem, blockCategory blockCat);

mesh *getMeshDefault          (item *cItem);
int   blockDamageDefault      (item *cItem, blockCategory blockCat);
bool  primaryActionDefault    (item *cItem, character *cChar, int to);
bool  secondaryActionDefault  (item *cItem, character *cChar, int to);
bool  tertiaryActionDefault   (item *cItem, character *cChar, int to);
bool  hasPrimaryActionDefault (item *cItem);
bool  isSingleItemDefault     (item *cItem);
float getInaccuracyDefault    (item *cItem);
int   getAmmunitionDefault    (item *cItem);
int   getStackSizeDefault     (item *cItem);