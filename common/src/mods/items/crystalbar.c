static const int ITEMID=269;

#include "../api_v1.h"

mesh *crystalbarGetMesh(const item *cItem){
	(void)cItem;
	(void)ITEMID;
	return meshCrystalbar;
}
