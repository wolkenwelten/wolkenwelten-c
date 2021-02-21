static const int ITEMID=287;

#include "../api_v1.h"

void firestickInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,1), itemNew(I_Stone,1));
	lispDefineID("i-","firestick",ITEMID);
}

int firestickGetStackSize(const item *cItem){
	(void)cItem;
	return 1;
}

bool firestickPrimaryAction(item *cItem,character *cChar){
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	ivec p = characterLOSBlock(cChar, 0);
	if(p.x < 0){return false;}

	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,3,600);
		fireNew(p.x,p.y,p.z,8);
	}
	return true;
}

bool firestickSecondaryAction(item *cItem,character *cChar){
	ivec p = characterLOSBlock(cChar, 1);
	if(p.x < 0){return false;}

	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,3,600);
		fireNew(p.x,p.y,p.z,8);
		return true;
	}
	return false;
}
