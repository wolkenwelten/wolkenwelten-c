static const int ITEMID=270;

#include "../api_v1.h"

bool crystalaxePrimaryAction(item *cItem, character *cChar){
	(void)ITEMID;
	if(throwableTry(cItem,cChar,0.25, 8, THROWABLE_PITCH_SPIN | THROWABLE_PIERCE)){return true;}
	return false;
}
