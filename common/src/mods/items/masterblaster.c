static const int ITEMID=262;

#include "../api_v1.h"

void masterblasterInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,16), itemNew(I_Iron_Bar,12), itemNew(I_Crystalbullet,16));
}

bool masterblasterPrimaryAction(item *cItem, character *cChar){
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	if(!characterTryToShoot(cChar,cItem,350,45)){return false;}
	beamblast(cChar,6.f,12.f,.5f,1024,1,32.f,1.f);
	return true;
}

int masterblasterItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}
