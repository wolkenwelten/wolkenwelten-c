static const int ITEMID=261;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"
#include "../../game/recipe.h"

void blasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,12, 13,6); // Crystal(12) + Hematite Ore(6) -> Blaster(1)
}

mesh *blasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool blasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool blasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool blasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 100){return false;}
	beamblast(cChar,0.75f,1.0f,0.15f,6);
	return true;
}