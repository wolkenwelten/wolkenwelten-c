#include "item.h"

#include "../game/blockType.h"
#include "../game/character.h"
#include "../game/grenade.h"
#include "../mods/mods.h"
#include "../voxel/bigchungus.h"


item itemNew(uint16_t ID, int16_t amount){
	item i;
	i.amount = MIN(99,amount);
	i.ID     = ID;
	return i;
}

item itemEmpty(){
	item i;
	i.amount = i.ID = 0;
	return i;
}

void itemDiscard(item *i){
	i->amount = i->ID = 0;
}

bool itemIsEmpty(item *i){
	return ((i->amount==0) || (i->ID==0));
}

int itemBlockDamage(item *i, blockCategory cat){
	return blockDamageDispatch(i,cat);
}

int itemCanStack(item *i, uint16_t ID){
	if(i->ID != ID)    {return 0;}
	if(i->amount >= 99){return 0;}
	if(i->amount ==  0){return 0;}
	return 99-i->amount;
}
int itemIncStack(item *i, int16_t amount){
	if((i->amount+amount)>99){amount = 99 - i->amount;}
	i->amount += amount;
	return amount;
}
int itemDecStack(item *i, int16_t amount){
	if(i->amount < amount){amount = i->amount;}
	i->amount -= amount;
	return amount;
}
