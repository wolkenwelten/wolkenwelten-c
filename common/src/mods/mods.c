#include "mods.h"

mesh *getMeshDefault(const item *cItem){
	int ID = cItem->ID;
	if(ID < 256){
		if(!blockTypeValid(ID)){ return NULL; }
		return blockTypeGetMesh(ID);
	}
	return NULL;
}
int damageDefault(const item *cItem){
	(void)cItem;

	return 1;
}
int blockDamageDefault(const item *cItem, blockCategory blockCat){
	(void)cItem;
	(void)blockCat;

	return 1;
}
bool primaryActionDefault(item *cItem, character *cChar){
	(void)cItem;
	(void)cChar;

	return false;
}
bool secondaryActionDefault(item *cItem, character *cChar){
	if((cItem->ID < 256) && blockTypeValid(cItem->ID)){
		return characterPlaceBlock(cChar, cItem);
	}
	return false;
}
bool tertiaryActionDefault(item *cItem, character *cChar){
	(void)cItem;
	(void)cChar;

	return false;
}
bool hasPrimaryActionDefault(const item *cItem){
	(void)cItem;

	return false;
}
float getInaccuracyDefault(const item *cItem){
	(void)cItem;

	return 4.f;
}
int getAmmunitionDefault(const item *cItem){
	int ID = cItem->ID;
	if(ID < 256){
		return ID;
	}
	return 0;
}
int getStackSizeDefault (const item *cItem){
	(void)cItem;

	return 99;
}
int getMagSizeDefault (const item *cItem){
	(void)cItem;

	return 0;
}

int itemDropCallbackDefault(const item *cItem, float x, float y, float z){
	(void)cItem;
	(void)x;
	(void)y;
	(void)z;
	return 0;
}
