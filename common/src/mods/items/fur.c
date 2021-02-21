static const int ITEMID=280;

#include "../api_v1.h"

void furInit(){
	lispDefineID("i-","fur",ITEMID);
}

int furItemDropCallback(const item *cItem, float x, float y, float z){
	(void)cItem;
	(void)x;
	(void)y;
	(void)z;
	if(rngValM(1<<16) == 0){ return -1;}
	return 0;
}
