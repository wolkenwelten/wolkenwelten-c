#include "mods.h"
#include "../game/item.h"
#include "../game/character.h"

inline int blockDamageDefault(const item *cItem, blockCategory blockCat){
	return 1;
}
inline bool activateItemDefault(item *cItem, character *cChar){
	return false;
}
inline bool mineActionDefault(item *cItem, character *cChar, int to){
	return false;
}
inline bool hasMineActionDefault(const item *cItem){
	return false;
}
inline bool isSingleItemDefault(const item *cItem){
	return false;
}

bool assblasterIsSingleItem(const item *cItem);
bool assblasterHasMineAction(const item *cItem);
bool assblasterMineAction(item *cItem, character *cChar, int to);

void axeInit();
int axeBlockDamage(const item *cItem, blockCategory blockCat);
bool axeIsSingleItem(const item *cItem);

void bombInit();
bool bombActivateItem(item *cItem,character *cChar);

void grenadeInit();
bool grenadeActivateItem(item *cItem,character *cChar);

bool pearActivateItem(item *cItem,character *cChar);

void pickaxeInit();
int pickaxeBlockDamage(const item *cItem, blockCategory blockCat);
bool pickaxeIsSingleItem(const item *cItem);

void modsInit(){
	grenadeInit();
	bombInit();
	axeInit();
	pickaxeInit();
}

int blockDamageDispatch(const item *cItem, blockCategory blockCat){
	switch(cItem->ID){
		case 259: return axeBlockDamage(cItem,blockCat);
		case 260: return pickaxeBlockDamage(cItem,blockCat);
	}
	return blockDamageDefault(cItem,blockCat);
}

bool isSingleItemDispatch(const item *cItem){
	switch(cItem->ID){
		case 259: return axeIsSingleItem(cItem);
		case 260: return pickaxeIsSingleItem(cItem);
		case 261: return assblasterIsSingleItem(cItem);
	}
	return isSingleItemDefault(cItem);
}

bool hasMineActionDispatch(const item *cItem){
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
