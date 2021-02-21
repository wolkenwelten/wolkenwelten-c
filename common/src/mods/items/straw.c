static const int ITEMID=293;

#include "../api_v1.h"

void strawInit(){
	lispDefineID("i-","straw",ITEMID);
}

int strawGetFireDmg(const itemDrop *id){
	(void)id;
	return 8;
}

int strawGetFireHealth(const itemDrop *id){
	(void)id;
	return 240;
}
