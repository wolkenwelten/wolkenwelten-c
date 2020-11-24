static const int ITEMID=266;

#include "../api_v1.h"

mesh *ironbarGetMesh(const item *cItem){
	(void)cItem;
	(void)ITEMID;
	return meshIronbar;
}
