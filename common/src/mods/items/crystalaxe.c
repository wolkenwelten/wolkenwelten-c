static const int ITEMID=270;

#include "../api_v1.h"

void crystalaxeInit(){
	recipeNew2(itemNew(ITEMID,1),itemNew(I_Iron_Axe,1),itemNew(I_Crystal_Bar,4));
	lispDefineID("i-","crystal axe",ITEMID);
}

bool crystalaxePrimaryAction(item *cItem, character *cChar){
	if(throwableTry(cItem,cChar,0.25, 8, THROWABLE_PITCH_SPIN | THROWABLE_PIERCE)){return true;}
	return false;
}

bool crystalaxeSecondaryAction(item *cItem, character *cChar){
	if(throwableTryAim(cItem,cChar)){return true;}
	return false;
}
