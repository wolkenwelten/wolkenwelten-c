static const int ITEMID=288;

#include "../api_v1.h"

bool waterthrowerPrimaryAction(item *cItem, character *cChar){
	(void)ITEMID;
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
