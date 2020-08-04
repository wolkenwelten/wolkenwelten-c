static const int ITEMID=264;

#include "../../gfx/mesh.h"
#include "../../gfx/objs.h"
#include "../../game/grenade.h"
#include "../../game/item.h"
#include "../../game/character.h"
#include "../../game/recipe.h"
#include "../../../../common/src/network/messages.h"
#include "../../../../common/src/misc/misc.h"

#include <stdio.h>

void shotgunblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,36, 13,18); // Crystal(36) + Hematite Ore(18) -> Shotgunblaster(1)
}

mesh *shotgunblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool shotgunblasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool shotgunblasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool shotgunblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 256){return false;}
	for(int i=48;i>0;i--){
		const float yaw   = cChar->yaw   + (rngValf()-0.5f)*8.f;
		const float pitch = cChar->pitch + (rngValf()-0.5f)*8.f;
		msgBeamBlast(cChar->x, cChar->y, cChar->z, yaw, pitch, 0.5f,1.f,0.04f,4);
	}
	return true;
}

bool shotgunblasterActivateItem(item *cItem,character *cChar, int to){
	(void)cItem;

	if(to < 96){return false;}
	for(int i=32;i>0;i--){
		const float yaw   = cChar->yaw   + (rngValf()-0.5f)*48.f;
		const float pitch = cChar->pitch + (rngValf()-0.5f)*48.f;
		msgBeamBlast(cChar->x, cChar->y, cChar->z, yaw, pitch, 0.5f,1.f,0.04f,4);
	}
	return true;
}
