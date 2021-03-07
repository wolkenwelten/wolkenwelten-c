static const int ITEMID=280;

#include "../api_v1.h"

int furItemDropCallback(const item *cItem, float x, float y, float z){
	(void)ITEMID;
	(void)cItem;
	(void)x;
	(void)y;
	(void)z;
	if(rngValM(1<<16) == 0){ return -1;}
	return 0;
}
