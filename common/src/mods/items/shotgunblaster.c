static const int ITEMID=264;

#include "../api_v1.h"

bool shotgunblasterPrimaryAction(item *cItem, character *cChar){
	(void)ITEMID;
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	if(!characterTryToShoot(cChar,cItem,128,6)){return false;}
	sfxPlay(sfxPhaser,0.5f);
	for(int i=0;i<64;i++){
		projectileNewC(cChar, 0, 4);
	}
	characterAddInaccuracy(cChar,32.f);
	characterAddRecoil(cChar,3.f);
	return true;
}

int shotgunblasterItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}
