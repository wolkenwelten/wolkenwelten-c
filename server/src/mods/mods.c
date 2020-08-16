#include "mods.h"

#include <stdlib.h>

mesh *getMeshDefault(item *cItem){
	(void)cItem;

	return NULL;
}
int blockDamageDefault(item *cItem, blockCategory blockCat){
	(void)cItem;
	(void)blockCat;

	return 1;
}
bool activateItemDefault(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;
	(void)to;

	return false;
}
bool mineActionDefault(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;
	(void)to;

	return false;
}
bool hasMineActionDefault(item *cItem){
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