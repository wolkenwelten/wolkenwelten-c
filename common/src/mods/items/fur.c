static const int ITEMID=280;

#include "../api_v1.h"

mesh *furGetMesh(const item *cItem){
	(void)cItem;

	return meshFur;
}

int furGetAmmunition(const item *cItem){
	(void)cItem;

	return ITEMID;
}
