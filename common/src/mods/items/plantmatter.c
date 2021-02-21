static const int ITEMID=292;

#include "../api_v1.h"

void plantmatterInit(){
	lispDefineID("i-","plantmatter",ITEMID);
}

int plantmatterItemDropCallback(const item *cItem, float x, float y, float z){
	(void)cItem;
	if(rngValA(65535) != 0){return 0;}
	item straw = itemNew(I_Straw,1);
	itemDropNewP(vecNew(x,y,z),&straw);
	return -1;
}
