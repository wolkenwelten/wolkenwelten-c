static const int ITEMID=277;

#include "../api_v1.h"

bool poopSecondaryAction(item *cItem,character *cChar, int to){
	(void)cItem;
	(void)ITEMID;

	if(to < 0){return false;}
	if(characterGetHP(cChar) >= characterGetMaxHP(cChar)){return false;}
	if(itemDecStack(cItem,1)){
		characterHP(cChar,-2);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,4,600);
		sfxPlay(sfxChomp,1.f);
		return true;
	}
	return false;
}

mesh *poopGetMesh(const item *cItem){
	(void)cItem;

	return meshPoop;
}

int poopItemDropCallback(const item *cItem, float x, float y, float z){
	(void)cItem;
	u8 b = worldGetB(x,y-0.5f,z);
	if(b != I_Dirt){
		if(b == 0){return 0;}
		if(rngValM(1024) == 0){
			return -1;
		}
		return 0;
	}
	if(rngValM(128) == 0){
		worldSetB(x,y-0.5f,z,I_Grass);
		return -1;
	}
	return 0;
}
