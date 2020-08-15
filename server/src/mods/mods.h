#pragma once
#include "../game/blockType.h"
#include "../game/item.h"
#include "../game/character.h"

struct mesh;
typedef struct mesh mesh;

int   blockDamageDispatch (item *cItem, blockCategory blockCat);

mesh *getMeshDefault     (item *cItem);
int blockDamageDefault   (item *cItem, blockCategory blockCat);
bool activateItemDefault (item *cItem, character *cChar, int to);
bool mineActionDefault   (item *cItem, character *cChar, int to);
bool hasMineActionDefault(item *cItem);
bool isSingleItemDefault (item *cItem);