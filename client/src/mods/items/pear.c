static const int ITEMID=258;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/item.h"
#include "../../game/character.h"

bool pearActivateItem(item *cItem,character *cChar){
	if(cChar->hp == cChar->maxhp){return false;}
	if(itemDecStack(cItem,1)){
		characterHP(cChar,4);
		return true;
	}
	return false;
}

mesh *pearGetMesh(item *cItem){
	return meshPear;
}
