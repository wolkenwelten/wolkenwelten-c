static const int ITEMID=268;

#include "../api_v1.h"

void ironpickaxeInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Stone_Pick,1), itemNew(I_Iron_Bar,4));
	lispDefineID("i-","iron pickaxe",ITEMID);
}
