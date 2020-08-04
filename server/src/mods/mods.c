#include "mods.h"
#include "../game/item.h"
#include "../game/character.h"

inline int blockDamageDefault(const item *cItem, blockCategory blockCat){
	(void)cItem;
	(void)blockCat;

	return 1;
}

int axeBlockDamage(const item *cItem, blockCategory blockCat);
int pickaxeBlockDamage(const item *cItem, blockCategory blockCat);

int blockDamageDispatch(const item *cItem, blockCategory blockCat){
	switch(cItem->ID){
		case 259: return axeBlockDamage(cItem,blockCat);
		case 260: return pickaxeBlockDamage(cItem,blockCat);
	}
	return blockDamageDefault(cItem,blockCat);
}
