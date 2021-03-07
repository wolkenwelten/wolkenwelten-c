static const int ITEMID=263;

#include "../api_v1.h"

bool assaultblasterPrimaryAction(item *cItem, character *cChar){
	(void)ITEMID;
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	if(!characterTryToShoot(cChar,cItem,15,1)){return false;}
	sfxPlay(sfxPhaser,0.2f);
	projectileNewC(cChar, 0, 1);
	characterAddInaccuracy(cChar,7.f);
	characterAddRecoil(cChar,1.f);
	return true;
}
