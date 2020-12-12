static const int ITEMID=256;

#include "../api_v1.h"

void grenadeInit(){
	recipeNew3(itemNew(ITEMID,3), itemNew(I_Iron_Bar,1), itemNew(I_Coal, 4), itemNew(I_Crystal, 1));
}

bool grenadeSecondaryAction(item *cItem,character *cChar){
	if(characterTryToUse(cChar,cItem,200,1)){
		grenadeNew(vecAdd(cChar->pos,vecNew(0,0.5f,0)),cChar->rot,1,0,0);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,0,300);
		return true;
	}
	return false;
}

mesh *grenadeGetMesh(const item *cItem){
	(void)cItem;
	return meshGrenade;
}

int grenadeGetAmmunition(const item *cItem){
	(void)cItem;
	return ITEMID;
}

int grenadeItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 1*id->itm.amount, 0);
	return 0;
}

int grenadeGetFireDmg(const itemDrop *id){
	(void)id;
	return 8;
}

int grenadeGetFireHealth(const itemDrop *id){
	(void)id;
	return 32;
}
