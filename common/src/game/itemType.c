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
#include "itemType.h"

#include "../misc/mesh.h"
#include "../../nujel/lib/casting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

itemType itemTypes[256];

void itemTypeInit(){
	for(int i=0;i<256;i++){
		itemTypes[i].stackSize  = 99;
		itemTypes[i].fireHealth = 256;
		itemTypes[i].fireDamage = 1;
		itemTypes[i].inaccuracy = 8.f;
		for(int ii=0;ii<5;ii++){itemTypes[i].damage[ii] = 1;}
		for(int ii=0;ii<4;ii++){
			itemTypes[i].spriteIndex[ii] = -1;
			itemTypes[i].spriteColor[ii] = 0xFFFFFFFF;
		}
	}
}

static lVal *wwlnfITName(lClosure *c, lVal *v){
	int   itemID     = -1;
	const char *name = NULL;

	v = getLArgI(c,v,&itemID);
	v = getLArgS(c,v,&name);

	if(itemID < 256){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(name != NULL){
		snprintf(it->name,32,"%s",name);
	}

	return lValString(it->name);
}

static lVal *wwlnfITMesh(lClosure *c, lVal *v){
	int itemID = -1;
	int meshID = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&meshID);

	if(itemID < 256){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(meshID >= 0){
		it->iMesh = meshGet(meshID);
	}

	return lValInt(meshIndex(it->iMesh));
}

static lVal *wwlnfITDamage(lClosure *c, lVal *v){
	int itemID = -1;
	int cat    = -1;
	int damage = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&cat);
	v = getLArgI(c,v,&damage);

	if((itemID < 256) || (cat < 0) || (cat > 4)){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(damage >= 0){
		it->damage[cat] = damage;
	}

	return lValInt(it->damage[cat]);
}

static lVal *wwlnfITAmmunition(lClosure *c, lVal *v){
	int itemID = -1;
	int ammunition = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&ammunition);

	if(itemID < 256){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(ammunition >= 0){
		it->ammunition = ammunition;
	}

	return lValInt(it->ammunition);
}

static lVal *wwlnfITStackSize(lClosure *c, lVal *v){
	int itemID    = -1;
	int stackSize = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&stackSize);

	if(itemID < 256){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(stackSize >= 0){
		it->stackSize = stackSize;
	}

	return lValInt(it->stackSize);
}

static lVal *wwlnfITMagazineSize(lClosure *c, lVal *v){
	int itemID       = -1;
	int magazineSize = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&magazineSize);

	if(itemID < 256){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(magazineSize >= 0){
		it->magazineSize = magazineSize;
	}

	return lValInt(it->magazineSize);
}

static lVal *wwlnfITFireDamage(lClosure *c, lVal *v){
	int itemID     = -1;
	int fireDamage = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&fireDamage);

	if(itemID < 256){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(fireDamage >= 0){
		it->fireDamage = fireDamage;
	}

	return lValInt(it->fireDamage);
}

static lVal *wwlnfITFireHealth(lClosure *c, lVal *v){
	int itemID     = -1;
	int fireHealth = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&fireHealth);

	if(itemID < 256){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(fireHealth >= 0){
		it->fireHealth = fireHealth;
	}

	return lValInt(it->fireHealth);
}

static lVal *wwlnfITInaccuracy(lClosure *c, lVal *v){
	int itemID  = -1;
	float inacc = -1.f;

	v = getLArgI(c,v,&itemID);
	v = getLArgF(c,v,&inacc);

	if(itemID < 256){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(inacc >= 0.f){
		it->inaccuracy = inacc;
	}

	return lValFloat(it->inaccuracy);
}

static lVal *wwlnfITIDChance(lClosure *c, lVal *v){
	int itemID = -1;
	int chance = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&chance);

	if(itemID < 256){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(chance >= 0){
		it->itemDropChance = chance;
	}

	return lValInt(it->itemDropChance);
}

static lVal *wwlnfITSpriteID(lClosure *c, lVal *v){
	int itemID      = -1;
	int i           = -1;
	int spriteIndex = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&i);
	v = getLArgI(c,v,&spriteIndex);

	if((itemID < 256) || (i < 0) || (i > 3)){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	if(spriteIndex >= 0){
		it->spriteIndex[i] = spriteIndex;
	}
	return lValInt(it->spriteIndex[i]);
}

static lVal *wwlnfITSpriteColor(lClosure *c, lVal *v){
	int itemID   = -1;
	int i        = -1;
	int color    = -1;

	v = getLArgI(c,v,&itemID);
	v = getLArgI(c,v,&i);
	v = getLArgI(c,v,&color);

	if((itemID < 256) || (i < 0) || (i > 3)){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	it->spriteColor[i] = color;
	return NULL;
}

static lVal *wwlnfITWeight(lClosure *c, lVal *v){
	int itemID   = -1;
	float weight = -1.f;

	v = getLArgI(c,v,&itemID);
	v = getLArgF(c,v,&weight);

	if((itemID < 256) || (weight < 0)){return NULL;}
	itemType *it = &itemTypes[itemID-256];
	it->weight = weight;
	return NULL;
}

void itemTypeLispClosure(lClosure *c){
	lAddNativeFunc(c,"it-name",        "(id &n)",       "Set name of itemType ID to &n if passed",                 wwlnfITName);
	lAddNativeFunc(c,"it-mesh",        "(id &m)",       "Set mesh of itemType ID to &m if passed",                 wwlnfITMesh);
	lAddNativeFunc(c,"it-ammunition",  "(id &a)",       "Set ammunition of itemType ID to &a if passed",           wwlnfITAmmunition);
	lAddNativeFunc(c,"it-stack-size",  "(id &s)",       "Set stackSize of itemType ID to &d if passed",            wwlnfITStackSize);
	lAddNativeFunc(c,"it-mag-size",    "(id &s)",       "Set Magazine Size of itemType ID to &d if passed",        wwlnfITMagazineSize);
	lAddNativeFunc(c,"it-damage" ,     "(id cat &d)",   "Set damage to cat blocks of itemType ID to &d if passed", wwlnfITDamage);
	lAddNativeFunc(c,"it-fire-damage", "(id &d)",       "Set fire damage of itemType ID to &d if passed",          wwlnfITFireDamage);
	lAddNativeFunc(c,"it-fire-health", "(id &h)",       "Set fire health of itemType ID to &h if passed",          wwlnfITFireHealth);
	lAddNativeFunc(c,"it-inaccuracy",  "(id &a)",       "Set fire inaccuracy of itemType ID to &a if passed",      wwlnfITInaccuracy);
	lAddNativeFunc(c,"it-item-drop-cb","(id &chance)",  "Set chance mask for the itemDrop callback to execute",    wwlnfITIDChance);
	lAddNativeFunc(c,"it-sprite-id",   "(id i &sprite)","Set sprite id I of ID to &SPRITE",                        wwlnfITSpriteID);
	lAddNativeFunc(c,"it-sprite-color","(id i &color)", "Set sprite color of ID to &COLOR",                        wwlnfITSpriteColor);
	lAddNativeFunc(c,"it-weight",      "(id weight)",   "Set the WEIGHT of ID",                                    wwlnfITWeight);
}
