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
	(void)cItem;
	(void)blockCat;

	return 1;
}
inline bool activateItemDefault(item *cItem, character *cChar){
	(void)cItem;
	(void)cChar;

	return false;
}
inline bool mineActionDefault(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)cChar;
	(void)to;

	return false;
}
inline bool hasMineActionDefault(item *cItem){
	(void)cItem;

	return false;
}
inline bool isSingleItemDefault(item *cItem){
	(void)cItem;

	return false;
}

void masterblasterInit();
mesh *masterblasterGetMesh(item *cItem);
bool masterblasterIsSingleItem(item *cItem);
bool masterblasterHasMineAction(item *cItem);
bool masterblasterMineAction(item *cItem, character *cChar, int to);

void blasterInit();
mesh *blasterGetMesh(item *cItem);
bool blasterIsSingleItem(item *cItem);
bool blasterHasMineAction(item *cItem);
bool blasterMineAction(item *cItem, character *cChar, int to);

void assaultblasterInit();
mesh *assaultblasterGetMesh(item *cItem);
bool assaultblasterIsSingleItem(item *cItem);
bool assaultblasterHasMineAction(item *cItem);
bool assaultblasterMineAction(item *cItem, character *cChar, int to);

void shotgunblasterInit();
mesh *shotgunblasterGetMesh(item *cItem);
bool shotgunblasterIsSingleItem(item *cItem);
bool shotgunblasterHasMineAction(item *cItem);
bool shotgunblasterMineAction(item *cItem, character *cChar, int to);
bool shotgunblasterActivateItem(item *cItem, character *cChar, int to);

void axeInit();
int axeBlockDamage(item *cItem, blockCategory blockCat);
mesh *axeGetMesh(item *cItem);
bool axeIsSingleItem(item *cItem);

void bombInit();
bool bombActivateItem(item *cItem,character *cChar, int to);
mesh *bombGetMesh(item *cItem);

void grenadeInit();
bool grenadeActivateItem(item *cItem,character *cChar, int to);
mesh *grenadeGetMesh(item *cItem);

void pearInit();
bool pearActivateItem(item *cItem,character *cChar, int to);
mesh *pearGetMesh(item *cItem);

void pickaxeInit();
int pickaxeBlockDamage(item *cItem, blockCategory blockCat);
mesh *pickaxeGetMesh(item *cItem);
bool pickaxeIsSingleItem(item *cItem);

void modsInit(){
	blasterInit();
	masterblasterInit();
	assaultblasterInit();
	shotgunblasterInit();
	grenadeInit();
	bombInit();
	axeInit();
	pickaxeInit();
	pearInit();
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
		case 261: return blasterGetMesh(cItem);
		case 262: return masterblasterGetMesh(cItem);
		case 263: return assaultblasterGetMesh(cItem);
		case 264: return shotgunblasterGetMesh(cItem);
	}
	return getMeshDefault(cItem);
}

bool isSingleItemDispatch(item *cItem){
	switch(cItem->ID){
		case 259: return axeIsSingleItem(cItem);
		case 260: return pickaxeIsSingleItem(cItem);
		case 261: return blasterIsSingleItem(cItem);
		case 262: return masterblasterIsSingleItem(cItem);
		case 263: return assaultblasterIsSingleItem(cItem);
		case 264: return shotgunblasterIsSingleItem(cItem);
	}
	return isSingleItemDefault(cItem);
}

bool hasMineActionDispatch(item *cItem){
	switch(cItem->ID){
		case 261: return blasterHasMineAction(cItem);
		case 262: return masterblasterHasMineAction(cItem);
		case 263: return assaultblasterHasMineAction(cItem);
		case 264: return shotgunblasterHasMineAction(cItem);
	}
	return hasMineActionDefault(cItem);
}

bool mineActionDispatch(item *cItem, character *cChar, int to){
	switch(cItem->ID){
		case 261: return blasterMineAction(cItem,cChar,to);
		case 262: return masterblasterMineAction(cItem,cChar,to);
		case 263: return assaultblasterMineAction(cItem,cChar,to);
		case 264: return shotgunblasterMineAction(cItem,cChar,to);
	}
	return mineActionDefault(cItem,cChar,to);
}

bool activateItemDispatch(item *cItem, character *cChar, int to){
	switch(cItem->ID){
		case 256: return grenadeActivateItem(cItem,cChar,to);
		case 257: return bombActivateItem(cItem,cChar,to);
		case 258: return pearActivateItem(cItem,cChar,to);
		case 264: return shotgunblasterActivateItem(cItem,cChar,to);
	}
	return activateItemDefault(cItem,cChar);
}
