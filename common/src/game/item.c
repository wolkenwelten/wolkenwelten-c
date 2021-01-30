/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "item.h"

#include "../mods/mods.h"

#include <stddef.h>

item itemNew(u16 ID, i16 amount){
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

bool itemIsEmpty(const item *i){
	return ((i == NULL) || (i->amount==0) || (i->ID==0));
}

int itemCanStack(const item *i, u16 ID){
	if(i == NULL)      {return 0;}
	const int ma = getStackSizeDispatch(i);
	if(ma == 1)        {return 0;}
	if(i->ID != ID)    {return 0;}
	if(i->amount >= ma){return 0;}
	if(i->amount ==  0){return 0;}
	return ma-i->amount;
}
int itemIncStack(item *i, i16 amount){
	if(i == NULL)      {return 0;}
	const int ma = getStackSizeDispatch(i);
	if((i->amount+amount)>ma){amount = ma - i->amount;}
	i->amount += amount;
	return amount;
}
int itemDecStack(item *i, i16 amount){
	if((i == NULL) || (amount == 0)){return 0;}
	if(getStackSizeDispatch(i) == 1){
		itemDiscard(i);
		return 1;
	}else{
		if(i->amount < amount){amount = i->amount;}
		i->amount -= amount;
		return amount;
	}
}
int itemGetAmmo(const item *i){
	if(i == NULL)      {return 0;}
	return i->amount-1;
}
int itemIncAmmo(item *i, i16 amount){
	if(i == NULL)      {return 0;}
	const int ma = getMagSizeDispatch(i)+1;
	if((i->amount+amount)>ma){amount = ma - i->amount;}
	i->amount += amount;
	return amount;
}
int itemDecAmmo(item *i, i16 amount){
	if(i == NULL)      {return 0;}
	if((i->amount-1) < amount){amount = i->amount-1;}
	i->amount -= amount;
	return amount;
}
