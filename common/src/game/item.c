#include "item.h"
#include "../mods/mods.h"

item itemNew(uint16_t ID, int16_t amount){
	item i;
	i.ID     = ID;
	i.amount = MIN(getStackSizeDispatch(&i),amount);
	return i;
}

item itemEmpty(){
	item i;
	i.amount = i.ID = 0;
	return i;
}

void itemDiscard(item *i){
	if(i == NULL)      {return;}
	i->amount = i->ID = 0;
}

bool itemIsEmpty(item *i){
	return ((i == NULL) || (i->amount==0) || (i->ID==0));
}

int itemCanStack(item *i, uint16_t ID){
	if(i == NULL)      {return 0;}
	const int ma = getStackSizeDispatch(i);
	if(ma == 1)        {return 0;}
	if(i->ID != ID)    {return 0;}
	if(i->amount >= ma){return 0;}
	if(i->amount ==  0){return 0;}
	return ma-i->amount;
}
int itemIncStack(item *i, int16_t amount){
	if(i == NULL)      {return 0;}
	const int ma = getStackSizeDispatch(i);
	if((i->amount+amount)>ma){amount = ma - i->amount;}
	i->amount += amount;
	return amount;
}
int itemDecStack(item *i, int16_t amount){
	if(i == NULL)      {return 0;}
	if(i->amount < amount){amount = i->amount;}
	i->amount -= amount;
	return amount;
}
int itemGetAmmo(item *i){
	if(i == NULL)      {return 0;}
	return i->amount-1;
}
int itemIncAmmo(item *i, int16_t amount){
	if(i == NULL)      {return 0;}
	const int ma = getMagSizeDispatch(i)+1;
	if((i->amount+amount)>ma){amount = ma - i->amount;}
	i->amount += amount;
	return amount;
}
int itemDecAmmo(item *i, int16_t amount){
	if(i == NULL)      {return 0;}
	if((i->amount-1) < amount){amount = i->amount-1;}
	i->amount -= amount;
	return amount;
}
