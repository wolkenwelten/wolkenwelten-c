static const int ITEMID=288;

#include "../api_v1.h"

void waterthrowerInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,8), itemNew(I_Iron_Bar,8), itemNew(I_Crystalbullet,8));
	lispDefineID("i-","waterthrower",ITEMID);
}

bool waterthrowerPrimaryAction(item *cItem, character *cChar){
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	if(!characterTryToShoot(cChar,cItem,15,1)){return false;}
	sfxPlay(sfxPhaser,0.2f);
	for(uint i=0;i<4;i++){
		projectileNewC(cChar, 0, 6);
	}
	characterAddInaccuracy(cChar,7.f);
	characterAddRecoil(cChar,1.f);
	return true;
}

bool waterthrowerSecondaryAction(item *cItem, character *cChar){
	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterToggleAim(cChar,2.f);
		characterAddInaccuracy(cChar,32.f);
		return true;
	}
	return false;
}

bool waterthrowerTertiaryAction(item *cItem, character *cChar){
	return characterItemReload(cChar, cItem, 50);
}
