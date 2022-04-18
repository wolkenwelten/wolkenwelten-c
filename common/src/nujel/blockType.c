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
#include "blockType.h"
#include "../../../common/src/game/blockType.h"

static lVal *wwlnfBlockTypeCountGet(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(blockTypeMax + 1);
}

static lVal *wwlnfBlockTypeNameGet(lClosure *c, lVal *v){
	(void)c;
	const int b = castToInt(lCar(v),-1);
	if(b < 0){return NULL;}
	return lValString(blockTypeGetName(b));
}

static lVal *wwlnfBlockTypeFireHealthGet(lClosure *c, lVal *v){
	(void)c;
	const int b = castToInt(lCar(v),-1);
	if(b < 0){return NULL;}
	return lValInt(blockTypeGetFireHealth(b));
}

static lVal *wwlnfBlockTypeFireDamageGet(lClosure *c, lVal *v){
	(void)c;
	const int b = castToInt(lCar(v),-1);
	if(b < 0){return NULL;}
	return lValInt(blockTypeGetFireDamage(b));
}

static lVal *wwlnfBlockTypeWeightGet(lClosure *c, lVal *v){
	(void)c;
	const int b = castToInt(lCar(v),-1);
	if(b < 0){return NULL;}
	return lValInt(blockTypeGetWeight(b));
}

static lVal *wwlnfBlockTypeCategoryGet(lClosure *c, lVal *v){
	(void)c;
	const int b = castToInt(lCar(v),-1);
	if(b < 0){return NULL;}
	switch(blockTypeGetCat(b)){
	default:
		return lValSym(":none");
	case DIRT:
		return lValSym(":dirt");
	case STONE:
		return lValSym(":stone");
	case WOOD:
		return lValSym(":wood");
	case LEAVES:
		return lValSym(":leaves");
	}
}

void lOperatorsBlockType(lClosure *c){
	lAddNativeFunc(c,"block/count",       "[]",  "Return the amount of blockTypes known", wwlnfBlockTypeCountGet);
	lAddNativeFunc(c,"block/name",        "[id]","Return the name of the block ID", wwlnfBlockTypeNameGet);
	lAddNativeFunc(c,"block/category",    "[id]","Return the category of the block ID", wwlnfBlockTypeCategoryGet);
	lAddNativeFunc(c,"block/fire-health", "[id]","Return the fire health of the block ID", wwlnfBlockTypeFireHealthGet);
	lAddNativeFunc(c,"block/fire-dmg",    "[id]","Return the fire damage of the block ID", wwlnfBlockTypeFireDamageGet);
	lAddNativeFunc(c,"block/weight",      "[id]","Return the weight of the block ID", wwlnfBlockTypeWeightGet);
}
