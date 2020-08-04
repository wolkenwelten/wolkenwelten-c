static const int ITEMID=262;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/recipe.h"
#include "../../game/character.h"

void masterblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,24, 13,12); // Crystal(24) + Hematite Ore(12) -> Masterblaster(1)
}

mesh *masterblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool masterblasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool masterblasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool masterblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;


	if(to < 400){return false;}
	beamblast(cChar,3.f,8.f,2.f,1024,1,32.f,1.f);
	return true;
}
