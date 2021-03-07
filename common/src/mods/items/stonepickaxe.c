static const int ITEMID=260;

#include "../api_v1.h"

void stonepickaxeInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,4), itemNew(I_Stone,4));
}
