#include "../../../common/src/common.h"
#include "../../../common/src/mods/mods.h"

#include "../game/blockType.h"
#include "../game/character.h"

mesh *getMeshDefault(item *cItem){
	int ID = cItem->ID;
	if(ID < 256){
		if(!blockTypeValid(ID)){ return NULL; }
		return blockTypeGetMesh(ID);
	}
	return NULL;
}
int damageDefault(item *cItem){
	(void)cItem;

	return 1;
}
int blockDamageDefault(item *cItem, blockCategory blockCat){
	(void)cItem;
	(void)blockCat;

	return 1;
}
bool primaryActionDefault(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;
	(void)to;

	return false;
}
bool secondaryActionDefault(item *cItem, character *cChar, int to){
	if(cItem->ID < 256){
		return itemPlaceBlock(cItem,cChar, to);
	}
	return false;
}
bool tertiaryActionDefault(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;
	(void)to;

	return false;
}
bool hasPrimaryActionDefault(item *cItem){
	(void)cItem;

	return false;
}
float getInaccuracyDefault(item *cItem){
	(void)cItem;

	return 4.f;
}
int getAmmunitionDefault(item *cItem){
	int ID = cItem->ID;
	if(ID < 256){
		return ID;
	}
	return 0;
}
int getStackSizeDefault (item *cItem){
	(void)cItem;

	return 99;
}
int getMagSizeDefault(item *cItem){
	(void)cItem;
	return 0;
}
