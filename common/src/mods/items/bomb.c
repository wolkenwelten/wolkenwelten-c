static const int ITEMID=257;

#include "../api_v1.h"

void bombInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Iron_Bar,1), itemNew(I_Coal, 4), itemNew(I_Crystal, 1));
	lispDefineID("i-","bomb",ITEMID);
}

bool bombSecondaryAction(item *cItem,character *cChar){
	if(characterTryToUse(cChar,cItem,200,1)){
		grenadeNew(vecAdd(cChar->pos,vecNew(0,0.5f,0)),cChar->rot,3,0,0);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,0,240);
		return true;
	}
	return false;
}

int bombItemDropBurnUp(itemDrop *id){
	(void)id;
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 3*id->itm.amount, 0);
	return 0;
}
