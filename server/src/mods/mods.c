#include "../../../common/src/common.h"
#include "../../../common/src/mods/mods.h"

#include <stdlib.h>

mesh *getMeshDefault(const item *cItem){
	(void)cItem;

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
bool primaryActionDefault(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;
	(void)to;

	return false;
}
bool secondaryActionDefault(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;
	(void)to;

	return false;
}
bool tertiaryActionDefault(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;
	(void)to;

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
