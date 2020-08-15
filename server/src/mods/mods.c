#include "mods.h"

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
bool isSingleItemDefault(item *cItem){
	(void)cItem;

	return false;
}
float getInaccuracyDefault(item *cItem){
	(void)cItem;
	
	return 4.f;
}
