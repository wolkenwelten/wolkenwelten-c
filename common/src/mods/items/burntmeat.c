static const int ITEMID=281;

#include "../api_v1.h"

bool burntmeatSecondaryAction(item *cItem,character *cChar){
	(void)ITEMID;
	if(characterTryToUse(cChar,cItem,200,1)){
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,4,600);
		sfxPlay(sfxChomp,1.f);
		return true;
	}
	return false;
}
