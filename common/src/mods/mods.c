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

#include "mods.h"

mesh *getMeshDefault(const item *cItem){
	int ID = cItem->ID;
	if(ID < 256){
		if(!blockTypeValid(ID)){ return NULL; }
		return blockTypeGetMesh(ID);
	}
	return NULL;
}
int damageDefault(const item *cItem){
	(void)cItem;

	return 1;
}
int blockDamageDefault(const item *cItem, blockCategory blockCat){
	(void)cItem;
	(void)blockCat;

	return 1;
}
bool primaryActionDefault(item *cItem, character *cChar){
	if(throwableTry(cItem,cChar,0.1f, 1, 0)){return true;}
	return false;
}
bool secondaryActionDefault(item *cItem, character *cChar){
	if((cItem->ID < 256) && blockTypeValid(cItem->ID)){
		return characterPlaceBlock(cChar, cItem);
	}
	return false;
}
bool tertiaryActionDefault(item *cItem, character *cChar){
	(void)cItem;
	(void)cChar;

	return false;
}
bool hasPrimaryActionDefault(const item *cItem){
	(void)cItem;

	return false;
}
float getInaccuracyDefault(const item *cItem){
	(void)cItem;

	return 4.f;
}
int getAmmunitionDefault(const item *cItem){
	int ID = cItem->ID;
	if(ID < 256){
		return ID;
	}
	return 0;
}
int getStackSizeDefault (const item *cItem){
	(void)cItem;

	return 99;
}
int getMagSizeDefault (const item *cItem){
	(void)cItem;

	return 0;
}

int itemDropCallbackDefault(const item *cItem, float x, float y, float z){
	(void)cItem;
	(void)x;
	(void)y;
	(void)z;
	return 0;
}

int itemDropBurnUpDefault(itemDrop *id){
	(void)id;
	return 0;
}

int getFireDmgDefault(const itemDrop *id){
	if(id->itm.ID < 256){
		return blockTypeGetFireDmg(id->itm.ID);
	}
	return 1;
}

int getFireHealthDefault(const itemDrop *id){
	if(id->itm.ID < 256){
		return blockTypeGetHP(id->itm.ID);
	}
	return 300;
}

int throwActionDefault(item *cItem, character *cChar){
	if(throwableTryAim(cItem,cChar)){return true;}
	return false;
}

const char *getItemNameDefault(const item *cItem){
	if(cItem->ID < 256){return blockTypeGetName(cItem->ID);}
	return "Unknown Item";
}
