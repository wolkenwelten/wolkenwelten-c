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

#include "../game/blockType.h"
#include "../game/itemType.h"
#include "../misc/lisp.h"
#include "../mods/mods.h"

#include <stddef.h>

item itemNew(u16 ID, i16 amount){
	item i;
	i.ID     = ID;
	i.amount = MIN(itemGetStackSize(&i),amount);
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
	const int ma = itemGetStackSize(i);
	if(ma == 1)        {return 0;}
	if(i->ID != ID)    {return 0;}
	if(i->amount >= ma){return 0;}
	if(i->amount ==  0){return 0;}
	return ma-i->amount;
}

int itemIncStack(item *i, i16 amount){
	if(i == NULL)      {return 0;}
	const int ma = itemGetStackSize(i);
	if((i->amount+amount)>ma){amount = ma - i->amount;}
	i->amount += amount;
	return amount;
}

int itemDecStack(item *i, i16 amount){
	if((i == NULL) || (amount == 0)){return 0;}
	if(itemGetStackSize(i) == 1){
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
	const int ma = itemGetMagazineSize(i)+1;
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

const char *itemGetName(const item *i){
	if(i == NULL)  {return "NULL";}
	if(i->ID < 256){return blockTypeGetName(i->ID);}
	if(i->ID > 512){return "OVER";}
	return itemTypes[i->ID-256].name;
}

mesh *itemGetMesh(const item *i){
	if(i == NULL)  {return meshPear;}
	if(i->ID < 256){return blockTypeGetMesh(i->ID);}
	if(i->ID > 512){return meshBunny;}
	return itemTypes[i->ID - 256].iMesh;
}

int itemGetStackSize(const item *i){
	if((i == NULL) || (i->ID < 256) || (i->ID > 512)){return 99;}
	return itemTypes[i->ID - 256].stackSize;
}
int itemGetMagazineSize(const item *i){
	if((i == NULL) || (i->ID < 256) || (i->ID > 512)){return 0;}
	return itemTypes[i->ID - 256].magazineSize;
}
int itemGetAmmunition(const item *i){
	if((i == NULL) || (i->ID > 512)){return -1;}
	if(i->ID < 256){return i->ID;}
	return itemTypes[i->ID - 256].ammunition;
}
int itemGetFireDamage(const item *i){
	if((i == NULL) || (i->ID > 512)){return 1;}
	if(i->ID < 256){return blockTypeGetFireDamage(i->ID);}
	return itemTypes[i->ID - 256].fireDamage;
}
int itemGetFireHealth(const item *i){
	if((i == NULL) || (i->ID > 512)){return 1;}
	if(i->ID < 256){return blockTypeGetFireHealth(i->ID);}
	return itemTypes[i->ID - 256].fireHealth;
}
int itemGetDamage(const item *i, blockCategory cat){
	if((i == NULL) || (i->ID < 256) || (i->ID > 512)){return 1;}
	return itemTypes[i->ID - 256].damage[cat];
}
float itemGetInaccuracy(const item *i){
	if((i == NULL) || (i->ID < 256) || (i->ID > 512)){return 8.f;}
	return itemTypes[i->ID - 256].inaccuracy;
}

uint itemGetIDChance(const item *i){
	if((i == NULL) || (i->ID < 256) || (i->ID > 512)){return 0;}
	return itemTypes[i->ID - 256].itemDropChance;
}

bool itemDoPrimary(item *cItem, character *cChar){
	if((cItem == NULL) || (cChar == NULL)){return false;}
	if((cItem->ID < 256) && blockTypeValid(cItem->ID)){
		if(throwableTry(cItem,cChar,0.1f, 1, 0)){return true;}
		return false;
	}
	if((cItem->ID >= 256) && (cItem->ID < 512)){
		lVal *ret = lispCallFuncI("item-primary", cItem->ID);
		if((ret == NULL) || ((ret->type == ltBool) && (!ret->vBool))){return false;}
		return true;
	}
	return false;
}

bool itemDoSecondary(item *cItem, character *cChar){
	if((cItem == NULL) || (cChar == NULL)){return false;}
	if((cItem->ID < 256) && blockTypeValid(cItem->ID)){
		return characterPlaceBlock(cChar, cItem);
	}
	if((cItem->ID >= 256) && (cItem->ID < 512)){
		lVal *ret = lispCallFuncI("item-secondary", cItem->ID);
		if((ret == NULL) || ((ret->type == ltBool) && (!ret->vBool))){return false;}
		return true;
	}
	return false;
}

bool itemDoTertiary(item *cItem, character *cChar){
	if((cItem == NULL) || (cChar == NULL)){return false;}
	if((cItem->ID < 256) && blockTypeValid(cItem->ID)){
		return false;
	}
	if((cItem->ID >= 256) && (cItem->ID < 512)){
		lVal *ret = lispCallFuncI("item-tertiary", cItem->ID);
		if((ret == NULL) || ((ret->type == ltBool) && (!ret->vBool))){return false;}
		return true;
	}
	return false;
}
