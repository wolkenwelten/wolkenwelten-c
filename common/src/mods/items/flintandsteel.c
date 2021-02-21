static const int ITEMID=282;

#include "../api_v1.h"

void flintandsteelInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Iron_Bar,1), itemNew(I_Stone,1));
	lispDefineID("i-","flint and steel",ITEMID);
}

bool flintandsteelPrimaryAction(item *cItem,character *cChar){
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

bool flintandsteelSecondaryAction(item *cItem,character *cChar){
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
