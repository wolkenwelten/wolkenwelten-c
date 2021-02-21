static const int ITEMID=263;
static const int MAGSIZE=60;

#include "../api_v1.h"

void assaultblasterInit(){
	(void)ITEMID;
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,8), itemNew(I_Iron_Bar,8), itemNew(I_Flamebullet,8));
	lispDefineID("i-","assaultblaster",ITEMID);
}

bool assaultblasterPrimaryAction(item *cItem, character *cChar){
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	if(!characterTryToShoot(cChar,cItem,15,1)){return false;}
	sfxPlay(sfxPhaser,0.2f);
	projectileNewC(cChar, 0, 1);
	characterAddInaccuracy(cChar,7.f);
	characterAddRecoil(cChar,1.f);
	return true;
}

bool assaultblasterSecondaryAction(item *cItem, character *cChar){
	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterToggleAim(cChar,2.f);
		characterAddInaccuracy(cChar,32.f);
		return true;
	}
	return false;
}

bool assaultblasterTertiaryAction(item *cItem, character *cChar){
	return characterItemReload(cChar, cItem, 50);
}

int assaultblasterGetMagSize(const item *cItem){
	(void)cItem;

	return MAGSIZE;
}

int assaultblasterItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}

int assaultblasterGetFireDmg(const itemDrop *id){
	(void)id;
	return 6;
}

int assaultblasterGetFireHealth(const itemDrop *id){
	(void)id;
	return 64;
}
