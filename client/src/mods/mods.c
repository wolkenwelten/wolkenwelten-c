#include "mods.h"
#include "../game/item.h"
#include "../game/character.h"

inline mesh *getMeshDefault(item *cItem){
	int ID = cItem->ID;
	if(ID < 256){
		if(!blockTypeValid(ID)){ return NULL; }
		return blockTypeGetMesh(ID);
	}
	return NULL;
}
inline int blockDamageDefault(item *cItem, blockCategory blockCat){
	return 1;
}
inline bool activateItemDefault(item *cItem, character *cChar){
	return false;
}
inline bool mineActionDefault(item *cItem, character *cChar, int to){
	return false;
}
inline bool hasMineActionDefault(item *cItem){
	return false;
}
inline bool isSingleItemDefault(item *cItem){
	return false;
}

mesh *assblasterGetMesh(item *cItem);
bool assblasterIsSingleItem(item *cItem);
bool assblasterHasMineAction(item *cItem);
bool assblasterMineAction(item *cItem, character *cChar, int to);

void axeInit();
int axeBlockDamage(item *cItem, blockCategory blockCat);
mesh *axeGetMesh(item *cItem);
bool axeIsSingleItem(item *cItem);

void bombInit();
bool bombActivateItem(item *cItem,character *cChar);
mesh *bombGetMesh(item *cItem);

void grenadeInit();
bool grenadeActivateItem(item *cItem,character *cChar);
mesh *grenadeGetMesh(item *cItem);

bool pearActivateItem(item *cItem,character *cChar);
mesh *pearGetMesh(item *cItem);

void pickaxeInit();
int pickaxeBlockDamage(item *cItem, blockCategory blockCat);
mesh *pickaxeGetMesh(item *cItem);
bool pickaxeIsSingleItem(item *cItem);

void modsInit(){
	grenadeInit();
	bombInit();
	axeInit();
	pickaxeInit();
}

int blockDamageDispatch(item *cItem, blockCategory blockCat){
	switch(cItem->ID){
		case 259: return axeBlockDamage(cItem,blockCat);
		case 260: return pickaxeBlockDamage(cItem,blockCat);
	}
	return blockDamageDefault(cItem,blockCat);
}

mesh *getMeshDispatch(item *cItem){
	switch(cItem->ID){
		case 256: return grenadeGetMesh(cItem);
		case 257: return bombGetMesh(cItem);
		case 258: return pearGetMesh(cItem);
		case 259: return axeGetMesh(cItem);
		case 260: return pickaxeGetMesh(cItem);
		case 261: return assblasterGetMesh(cItem);
	}
	return getMeshDefault(cItem);
}

bool isSingleItemDispatch(item *cItem){
	switch(cItem->ID){
		case 259: return axeIsSingleItem(cItem);
		case 260: return pickaxeIsSingleItem(cItem);
		case 261: return assblasterIsSingleItem(cItem);
	}
	return isSingleItemDefault(cItem);
}

bool hasMineActionDispatch(item *cItem){
	switch(cItem->ID){
		case 261: return assblasterHasMineAction(cItem);
	}
	return hasMineActionDefault(cItem);
}

bool mineActionDispatch(item *cItem, character *cChar, int to){
	switch(cItem->ID){
		case 261: return assblasterMineAction(cItem,cChar,to);
	}
	return mineActionDefault(cItem,cChar,to);
}

bool activateItemDispatch(item *cItem, character *cChar){
	switch(cItem->ID){
		case 256: return grenadeActivateItem(cItem,cChar);
		case 257: return bombActivateItem(cItem,cChar);
		case 258: return pearActivateItem(cItem,cChar);
	}
	return activateItemDefault(cItem,cChar);
}
