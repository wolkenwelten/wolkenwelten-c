static const int ITEMID=258;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/item.h"
#include "../../game/character.h"

void pearInit(){
	(void)ITEMID;
}

bool pearActivateItem(item *cItem,character *cChar, int to){
	(void)cItem;
	if(to < 0){return false;}
	if(cChar->hp == cChar->maxhp){return false;}
	if(itemDecStack(cItem,1)){
		characterHP(cChar,4);
		characterAddCooldown(cChar,100);
		return true;
	}
	return false;
}

mesh *pearGetMesh(item *cItem){
	(void)cItem;

	return meshPear;
}
