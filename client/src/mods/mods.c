#include "mods.h"
#include "../game/item.h"
#include "../game/character.h"

mesh *getMeshDefault(item *cItem){
	int ID = cItem->ID;
	if(ID < 256){
		if(!blockTypeValid(ID)){ return NULL; }
		return blockTypeGetMesh(ID);
	}
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

