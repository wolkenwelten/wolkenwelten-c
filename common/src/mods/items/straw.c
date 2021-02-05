static const int ITEMID=293;

#include "../api_v1.h"

void strawInit(){
	lispDefineID("i-","straw",ITEMID);
}

mesh *strawGetMesh(const item *cItem){
	(void)ITEMID;
	(void)cItem;
	return meshStraw;
}

int strawGetFireDmg(const itemDrop *id){
	(void)id;
	return 8;
}

int strawGetFireHealth(const itemDrop *id){
	(void)id;
	return 240;
}
