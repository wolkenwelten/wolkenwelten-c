static const int ITEMID=273;

#include "../api_v1.h"

void clusterbombInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Iron_Bar,3), itemNew(I_Coal, 12), itemNew(I_Crystal, 4));
}

bool clusterbombSecondaryAction(item *cItem,character *cChar){
	if(characterTryToUse(cChar,cItem,200,1)){
		grenadeNew(vecAdd(cChar->pos,vecNew(0,0.5f,0)),cChar->rot,1,48,1.f);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,0,300);
		return true;
	}
	return false;
}

mesh *clusterbombGetMesh(const item *cItem){
	(void)cItem;

	return meshBomb;
}

int clusterbombGetAmmunition(const item *cItem){
	(void)cItem;

	return ITEMID;
}

int clusterbombItemDropBurnUp(itemDrop *id){
	(void)id;
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 4, 0);
	return 0;
}

int clusterbombGetFireDmg(const itemDrop *id){
	(void)id;
	return 8;
}

int clusterbombGetFireHealth(const itemDrop *id){
	(void)id;
	return 48;
}
