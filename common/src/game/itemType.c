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

#include "../mods/api_v1.h"
#include "../nujel/casting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

itemType itemTypes[256];

void itemTypeInit(){
	for(int i=0;i<256;i++){
		itemTypes[i].stackSize = 99;
		for(int ii=0;ii<5;ii++){itemTypes[i].damage[ii] = 1;}
	}
}

static lVal *wwlnfITName(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}

	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t->vInt < 256) || (t->vInt > 512)){return NULL;}
	const int ID = t->vInt;
	itemType *it = &itemTypes[ID-256];
	v = v->vList.cdr;

	if((v != NULL) && (v->type == ltPair)){
		t = lEval(c,v->vList.car);
		if(t != NULL){
			//printf("[%u]{%p} '%s' -> '%s'",ID,it,it->name,t->vString->data);
			snprintf(it->name,32,"%s",t->vString->data);
		}
	}
	return lValString(it->name);
}

static lVal *wwlnfITMesh(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}

	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t->vInt < 256) || (t->vInt > 512)){return NULL;}
	const int ID = t->vInt;
	itemType *it = &itemTypes[ID-256];
	v = v->vList.cdr;

	if((v != NULL) && (v->type == ltPair)){
		t = lnfInt(c,lEval(c,v->vList.car));
		if(t != NULL){it->iMesh = meshGet(t->vInt);}
	}
	return lValInt(meshIndex(it->iMesh));
}

static lVal *wwlnfITDamage(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}

	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t->vInt < 256) || (t->vInt > 512)){return NULL;}
	const int ID = t->vInt;
	itemType *it = &itemTypes[ID-256];
	v = v->vList.cdr;

	if((v == NULL) || (v->type != ltPair)){return NULL;}
	t = lnfInt(c,lEval(c,v->vList.car));
	const int dmgCat = t->vInt;
	if((dmgCat < 0) || (dmgCat >= 5)){return NULL;}
	v = v->vList.cdr;

	if((v != NULL) && (v->type == ltPair)){
		t = lnfInt(c,lEval(c,v->vList.car));
		if(t != NULL){it->damage[dmgCat] = t->vInt;}
	}
	return lValInt(it->damage[dmgCat]);
}

static lVal *wwlnfITAmmunition(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}

	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t->vInt < 256) || (t->vInt > 512)){return NULL;}
	const int ID = t->vInt;
	itemType *it = &itemTypes[ID-256];
	v = v->vList.cdr;

	if((v != NULL) && (v->type == ltPair)){
		t = lnfInt(c,lEval(c,v->vList.car));
		if(t != NULL){it->ammunition = t->vInt;}
	}
	return lValInt(it->ammunition);
}

static lVal *wwlnfITStackSize(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}

	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t->vInt < 256) || (t->vInt > 512)){return NULL;}
	const int ID = t->vInt;
	itemType *it = &itemTypes[ID-256];
	v = v->vList.cdr;

	if((v != NULL) && (v->type == ltPair)){
		t = lnfInt(c,lEval(c,v->vList.car));
		if(t != NULL){it->stackSize = t->vInt;}
	}
	return lValInt(it->stackSize);
}

static lVal *wwlnfITFireDamage(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}

	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t->vInt < 256) || (t->vInt > 512)){return NULL;}
	const int ID = t->vInt;
	itemType *it = &itemTypes[ID-256];
	v = v->vList.cdr;

	if((v != NULL) && (v->type == ltPair)){
		t = lnfInt(c,lEval(c,v->vList.car));
		if(t != NULL){it->fireDmg = t->vInt;}
	}
	return lValInt(it->fireDmg);
}

static lVal *wwlnfITFireHealth(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltPair)){return NULL;}

	lVal *t = lnfInt(c,lEval(c,v->vList.car));
	if((t->vInt < 256) || (t->vInt > 512)){return NULL;}
	const int ID = t->vInt;
	itemType *it = &itemTypes[ID-256];
	v = v->vList.cdr;

	if((v != NULL) && (v->type == ltPair)){
		t = lnfInt(c,lEval(c,v->vList.car));
		if(t != NULL){it->fireHealth = t->vInt;}
	}
	return lValInt(it->fireHealth);
}

void itemTypeLispClosure(lClosure *c){
	lAddNativeFunc(c,"it-name",       "(id &n)",    "Sets the name of itemType ID to &n if passed",                 wwlnfITName);
	lAddNativeFunc(c,"it-mesh",       "(id &m)",    "Sets the mesh of itemType ID to &m if passed",                 wwlnfITMesh);
	lAddNativeFunc(c,"it-ammunition", "(id &a)",    "Sets the ammunition of itemType ID to &a if passed",           wwlnfITAmmunition);
	lAddNativeFunc(c,"it-stack-size", "(id &s)",    "Sets the stackSize of itemType ID to &d if passed",            wwlnfITStackSize);
	lAddNativeFunc(c,"it-damage" ,    "(id cat &d)","Sets the damage to cat blocks of itemType ID to &d if passed", wwlnfITDamage);
	lAddNativeFunc(c,"it-fire-damage","(id &d)",    "Sets the fire damage of itemType ID to &d if passed",          wwlnfITFireDamage);
	lAddNativeFunc(c,"it-fire-health","(id &h)",    "Sets the fire health of itemType ID to &h if passed",          wwlnfITFireHealth);
}
