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

#include "character.h"
#include "../game/being.h"
#include "../game/item.h"
#include "../game/hook.h"
#include "../misc/sfx.h"
#include "../network/messages.h"
#include "../world/world.h"

#include <stdio.h>
#include <stddef.h>
#include <math.h>

character  characterList[64];
uint       characterCount = 0;

character *characterNew(){
	character *c = NULL;

	for(uint i=0;i<characterCount;i++){
		if(characterList[i].eMesh == NULL){
			c = &characterList[i];
			break;
		}
	}
	if(c == NULL){
		if(characterCount >= countof(characterList)){
			fprintf(stderr,"characterList Overflow!\n");
			return NULL;
		}
		c = &characterList[characterCount++];
	}
	characterInit(c);

	return c;
}

void characterFree(character *c){
	c->eMesh = NULL;
	c->hp = -1;
	if((c->hook != NULL) && isClient){
		hookFree(c->hook);
		c->hook = NULL;
	}
}

int characterGetHP(const character *c){
	return c->hp;
}

int characterGetMaxHP(const character *c){
	return c->maxhp;
}

void characterToggleGlider(character *c){
	if(c == NULL){return;}
	c->flags ^= CHAR_GLIDE;
}
void characterOpenGlider(character *c){
	if(c == NULL){return;}
	c->flags |= CHAR_GLIDE;
}
void characterCloseGlider(character *c){
	if(c == NULL){return;}
	c->flags &= ~CHAR_GLIDE;
}

void characterOpenConsMode   (      character *c){
	if(c->flags & (CHAR_AIMING | CHAR_THROW_AIM)){characterStopAim(c);}
	if(c->flags & (CHAR_GLIDE)){characterCloseGlider(c);}
	c->flags |= CHAR_CONS_MODE;
}

void characterCloseConsMode  (      character *c){
	if(c->flags & (CHAR_AIMING | CHAR_THROW_AIM)){characterStopAim(c);}
	if(c->flags & (CHAR_GLIDE)){characterCloseGlider(c);}
	c->flags &= ~CHAR_CONS_MODE;
}

void characterToggleConsMode (      character *c){
	if(c->flags & CHAR_CONS_MODE){
		characterCloseConsMode(c);
	}else{
		characterOpenConsMode(c);
	}
}

bool characterIsAiming(const character *c){
	if(c == NULL){return false;}
	return c->flags & CHAR_AIMING;
}
bool characterIsThrowAiming(const character *c){
	if(c == NULL){return false;}
	const uint mask = CHAR_AIMING | CHAR_THROW_AIM;
	return (c->flags & mask) == mask;
}
void characterToggleThrowAim(character *c, float zoom){
	if(c == NULL){return;}
	if(c->flags & CHAR_AIMING){
		characterStopAim(c);
		return;
	}
	c->goalZoomFactor = zoom;
	c->flags |= CHAR_AIMING;
	c->flags |= CHAR_THROW_AIM;
}
void characterToggleAim(character *c, float zoom){
	if(c == NULL){return;}
	c->flags ^= CHAR_AIMING;
	if(c->flags & CHAR_AIMING){
		c->goalZoomFactor = zoom;
	}else{
		c->goalZoomFactor = 1.f;
	}
}
void characterStopAim(character *c){
	if(c == NULL){return;}
	c->goalZoomFactor = 1.f;
	c->flags &= ~(CHAR_AIMING | CHAR_THROW_AIM);
}

void characterAddCooldown(character *c, int cooldown){
	c->actionTimeout = -cooldown;
}

void characterSetPos(character *c, const vec pos){
	c->pos = pos;
}

void characterSetRot(character *c, const vec rot){
	c->rot = rot;
}

void characterSetVelocity(character *c, const vec vel){
	c->vel = vel;
}

void characterSetInaccuracy(character *c, float inacc){
	c->inaccuracy = inacc;
}

void characterAddInaccuracy(character *c, float inc){
	c->inaccuracy += inc;
}

int characterGetItemAmount(const character *c, u16 itemID){
	int amount = 0;
	for(unsigned int i=0;i<CHAR_INV_MAX;i++){
		if(c->inventory[i].ID == itemID){
			amount += c->inventory[i].amount;
		}
	}
	return amount;
}

int characterDecItemAmount(character *c, u16 itemID,int amount){
	int ret=0;

	if(amount == 0){return 0;}
	for(uint i=0;i<CHAR_INV_MAX;i++){
		if(c->inventory[i].ID == itemID){
			if(c->inventory[i].amount > amount){
				itemDecStack(&c->inventory[i],amount);
				return amount;
			}else{
				amount -= c->inventory[i].amount;
				ret    += c->inventory[i].amount;
				c->inventory[i] = itemEmpty();
			}
		}
	}
	return ret;
}

int characterPickupItem(character *c, u16 itemID,int amount){
	int a = 0;
	item ci = itemNew(itemID,amount);
	if(itemGetStackSize(&ci) == 1){
		ci.amount = amount;
		for(uint i=0;i<CHAR_INV_MAX;i++){
			if(itemIsEmpty(&c->inventory[i])){
				c->inventory[i] = ci;
				sfxPlay(sfxPock,.8f);
				return 0;
			}
		}
		return -1;
	}

	for(uint i=0;i<CHAR_INV_MAX;i++){
		if(a >= amount){break;}
		if(itemCanStack(&c->inventory[i],itemID)){
			a += itemIncStack(&c->inventory[i],amount - a);
		}
	}
	for(uint i=0;i<CHAR_INV_MAX;i++){
		if(a >= amount){break;}
		if(itemIsEmpty(&c->inventory[i])){
			c->inventory[i] = itemNew(itemID,amount - a);
			a += c->inventory[i].amount;
		}
	}

	if(a == amount){
		sfxPlay(sfxPock,.8f);
		return 0;
	}else if(a != 0){
		sfxPlay(sfxPock,.8f);
	}else if(a == 0){
		return -1;
	}
	return a;
}

bool characterPlaceBlock(character *c,item *i){
	if(c->actionTimeout < 0)              { return false; }
	ivec los = characterLOSBlock(c,true);
	if(los.x < 0)                         { return false; }
	if(itemIsEmpty(i))                    { return false; }
	characterStartAnimation(c,0,240);
	characterAddCooldown(c,50);
	if((characterCollision(c->pos)&0xFF0)){
		sfxPlay(sfxStomp,1.f);
		const vec cvec = characterGetCollisionVec(c->pos);
		c->vel = vecAdd(c->vel,vecMulS(cvec,-0.01f));
		return false;
	}
	worldSetB(los.x,los.y,los.z,i->ID);
	if((characterCollision(c->pos)&0xFF0) != 0){
		const vec cvec = characterGetCollisionVec(c->pos);
		c->vel = vecAdd(c->vel,vecMulS(cvec,-0.01f));
		worldSetB(los.x,los.y,los.z,0);
		sfxPlay(sfxStomp,1.f);
		return false;
	} else {
		msgPlaceBlock(los.x,los.y,los.z,i->ID);
		itemDecStack(i,1);
		sfxPlay(sfxPock,1.f);
		return true;
	}
}

bool characterHP(character *c, int addhp){
	c->hp += addhp;
	if(c->hp <= 0){
		characterDie(c);
		return true;
	}
	if(c->hp > c->maxhp){
		c->hp = c->maxhp;
	}
	return false;
}

void characterEmptyInventory(character *c){
	for(uint i=0;i<CHAR_INV_MAX;i++){
		c->inventory[i] = itemEmpty();
	}
}

item *characterGetItemBarSlot(character *c, uint i){
	if(i >= CHAR_INV_MAX){return NULL;}
	return &c->inventory[i];
}

item *characterGetActiveItem(character *c){
	return characterGetItemBarSlot(c,c->activeItem);
}

void characterSetItemBarSlot(character *c, uint i, item *itm){
	if(i >= CHAR_INV_MAX){return;}
	c->inventory[i] = *itm;
}

void characterSetActiveItem(character *c,  int i){
	if(i > 9){i = 0;}
	if(i < 0){i = 9;}
	if((uint)i != c->activeItem){
		characterStartAnimation(c,5,200);
	}
	characterStopAim(c);
	c->activeItem = i;
}

void characterSwapItemSlots(character *c, uint a,uint b){
	if(a >= CHAR_INV_MAX){return;}
	if(b >= CHAR_INV_MAX){return;}

	item tmp = c->inventory[a];
	c->inventory[a] = c->inventory[b];
	c->inventory[b] = tmp;
}

ivec characterLOSBlock(const character *c, bool returnBeforeBlock) {
	const vec cv = vecMulS(vecDegToVec(c->rot),0.0625f);
	vec       cp = vecAdd(c->pos,vecNew(0,0.5,0));
	ivec      l  = ivecNewV(cp);

	for(int i=0;i<64;i++){
		cp = vecAdd(cp,cv);
		const ivec ip = ivecNewV(cp);
		if(!ivecEq(ip,l)){
			if(worldGetB(ip.x,ip.y,ip.z) > 0){
				return returnBeforeBlock ? l : ip;
			}
			l = ip;
		}
	}
	return ivecNOne();
}

u32 characterCollision(const vec c){
	u32 col = 0;
	const float wd = 0.4f;
	const float WD = 0.9f;

	if(checkCollision(c.x-wd,c.y+1.f,c.z   )) {col |=  0x10;}
	if(checkCollision(c.x+wd,c.y+1.f,c.z   )) {col |=  0x20;}
	if(checkCollision(c.x   ,c.y+1.f,c.z-wd)) {col |=  0x40;}
	if(checkCollision(c.x   ,c.y+1.f,c.z+wd)) {col |=  0x80;}

	if(checkCollision(c.x-wd,c.y-2.f ,c.z   )){col |=   0x1;}
	if(checkCollision(c.x+wd,c.y-2.f ,c.z   )){col |=   0x2;}
	if(checkCollision(c.x   ,c.y-2.f ,c.z-wd)){col |=   0x4;}
	if(checkCollision(c.x   ,c.y-2.f ,c.z+wd)){col |=   0x8;}

	if(checkCollision(c.x-WD,c.y-1.4f,c.z   )){col |= 0x100;}
	if(checkCollision(c.x+WD,c.y-1.4f,c.z   )){col |= 0x200;}
	if(checkCollision(c.x   ,c.y-1.4f,c.z-WD)){col |= 0x400;}
	if(checkCollision(c.x   ,c.y-1.4f,c.z+WD)){col |= 0x800;}

	if(checkCollision(c.x-WD,c.y-0.9f,c.z   )){col |= 0x1000;}
	if(checkCollision(c.x+WD,c.y-0.9f,c.z   )){col |= 0x2000;}
	if(checkCollision(c.x   ,c.y-0.9f,c.z-WD)){col |= 0x4000;}
	if(checkCollision(c.x   ,c.y-0.9f,c.z+WD)){col |= 0x8000;}

	return col;
}



u8 characterCollisionBlock(const vec c, vec *retPos){
	u8 b = 0;
	const float wd = 0.4f;
	const float WD = 0.9f;

	if((b = worldGetB(c.x-wd,c.y+1.f,c.z   ))) {*retPos = vecNew(c.x-wd,c.y+1.f,c.z   ); return b;}
	if((b = worldGetB(c.x+wd,c.y+1.f,c.z   ))) {*retPos = vecNew(c.x+wd,c.y+1.f,c.z   ); return b;}
	if((b = worldGetB(c.x   ,c.y+1.f,c.z-wd))) {*retPos = vecNew(c.x   ,c.y+1.f,c.z-wd); return b;}
	if((b = worldGetB(c.x   ,c.y+1.f,c.z+wd))) {*retPos = vecNew(c.x   ,c.y+1.f,c.z+wd); return b;}

	if((b = worldGetB(c.x-WD,c.y-0.9f,c.z   ))){*retPos = vecNew(c.x-WD,c.y-0.9f,c.z   ); return b;}
	if((b = worldGetB(c.x+WD,c.y-0.9f,c.z   ))){*retPos = vecNew(c.x+WD,c.y-0.9f,c.z   ); return b;}
	if((b = worldGetB(c.x   ,c.y-0.9f,c.z-WD))){*retPos = vecNew(c.x   ,c.y-0.9f,c.z-WD); return b;}
	if((b = worldGetB(c.x   ,c.y-0.9f,c.z+WD))){*retPos = vecNew(c.x   ,c.y-0.9f,c.z+WD); return b;}

	if((b = worldGetB(c.x-WD,c.y-1.4f,c.z   ))){*retPos = vecNew(c.x-WD,c.y-1.4f,c.z   ); return b;}
	if((b = worldGetB(c.x+WD,c.y-1.4f,c.z   ))){*retPos = vecNew(c.x+WD,c.y-1.4f,c.z   ); return b;}
	if((b = worldGetB(c.x   ,c.y-1.4f,c.z-WD))){*retPos = vecNew(c.x   ,c.y-1.4f,c.z-WD); return b;}
	if((b = worldGetB(c.x   ,c.y-1.4f,c.z+WD))){*retPos = vecNew(c.x   ,c.y-1.4f,c.z+WD); return b;}

	if((b = worldGetB(c.x-wd,c.y-2.f ,c.z   ))){*retPos = vecNew(c.x-wd,c.y-2.f,c.z   ); return b;}
	if((b = worldGetB(c.x+wd,c.y-2.f ,c.z   ))){*retPos = vecNew(c.x+wd,c.y-2.f,c.z   ); return b;}
	if((b = worldGetB(c.x   ,c.y-2.f ,c.z-wd))){*retPos = vecNew(c.x   ,c.y-2.f,c.z-wd); return b;}
	if((b = worldGetB(c.x   ,c.y-2.f ,c.z+wd))){*retPos = vecNew(c.x   ,c.y-2.f,c.z+wd); return b;}

	return 0;
}

vec characterGetCollisionVec(const vec pos){
	u32 col = characterCollision(pos);
	vec ret = vecNew(0.f,-0.8f,0.f);
	if(col & 0x110){ret.x -= 1.f;}
	if(col & 0x220){ret.x += 1.f;}
	if(col & 0x440){ret.z -= 1.f;}
	if(col & 0x880){ret.z += 1.f;}

	return ret;
}

void characterMove(character *c, const vec mov){
	const float yaw   = c->rot.yaw;

	if(c->flags & CHAR_NOCLIP){
		float s = 0.2f;
		if(c->flags & CHAR_BOOSTING){ s = 1.f;}
		c->gvel    = vecMulS(vecDegToVec(c->rot),mov.z*(-s));
		c->gvel.x += cosf((yaw)*PI/180)*mov.x*s;
		c->gvel.z += sinf((yaw)*PI/180)*mov.x*s;
		c->gvel.y += mov.y;
	}else{
		float s;
		if(c->flags & CHAR_AIMING){
			s = 0.01f;
		}else if(c->flags & CHAR_CONS_MODE){
			s = 0.025f;
		}else{
			s = 0.05f;
		}
		c->gvel.x = (cosf((yaw+90)*PI/180)*mov.z*s);
		c->gvel.z = (sinf((yaw+90)*PI/180)*mov.z*s);

		c->gvel.x += cosf((yaw)*PI/180)*mov.x*s;
		c->gvel.z += sinf((yaw)*PI/180)*mov.x*s;
		c->inaccuracy += s*78.f*vecMag(mov);
	}
}

void characterRotate(character *c, const vec rot){
	c->rot = vecAdd(c->rot,rot);
	c->inaccuracy += vecAbsSum(rot) * c->zoomFactor;

	if(c->rot.pitch < -90.f){
		 c->rot.pitch = -90.f;
	}else if(c->rot.pitch >  90.f){
		 c->rot.pitch =  90.f;
	}
	if(c->rot.yaw < 0.f){
		 c->rot.yaw += 360.f;
	}else if(c->rot.yaw >  360.f){
		 c->rot.yaw -= 360.f;
	}
}

void characterStopAnimation(character *c){
	c->animationIndex     = 0;
	c->animationTicksLeft = 0;
}

void characterStartAnimation(character *c, int index, int duration){
	c->animationIndex     = index;
	c->animationTicksLeft = c->animationTicksMax = duration;
}

bool characterItemReload(character *c, item *i, int cooldown){
	const int MAGSIZE = itemGetMagazineSize(i);
	const int AMMO    = itemGetAmmunition(i);
	int ammoleft      = characterGetItemAmount(c,AMMO);

	if(c->actionTimeout < 0)      {return false;}
	if(itemGetAmmo(i) == MAGSIZE) {return false;}
	if(ammoleft <= 0)             {return false;}

	ammoleft = MIN(MAGSIZE,ammoleft);
	characterDecItemAmount(c, AMMO, itemIncAmmo(i,ammoleft));

	characterAddCooldown(c,cooldown);
	sfxPlay(sfxHookReturned,1.f);
	characterStartAnimation(c,2,MAX(cooldown*2,500));

	return true;
}

bool characterTryToShoot(character *c, item *i, int cooldown, int bulletcount){
	if(c->actionTimeout < 0){return false;}
	if(itemGetAmmo(i) < bulletcount){
		if(characterItemReload(c,i,256)){return false;}
		sfxPlay(sfxHookFire,0.3f);
		characterAddCooldown(c,64);
		characterStartAnimation(c,3,250);
		return false;
	}
	itemDecAmmo(i,bulletcount);
	characterAddCooldown(c,cooldown);
	characterStartAnimation(c,1,250);
	return true;
}

bool characterTryToUse(character *c, item *i, int cooldown, int itemcount){
	if(c->actionTimeout < 0){return false;}
	if(i->amount < itemcount){
		sfxPlay(sfxHookFire,0.3f);
		characterAddCooldown(c,64);
		return false;
	}
	itemDecStack(i,itemcount);
	characterAddCooldown(c,cooldown);
	return true;
}

void characterSetInventoryP(character *c, const packet *p){
	if(c == NULL){return;}
	uint max = MIN(CHAR_INV_MAX,packetLen(p)/4);
	uint ii = 0;
	for(uint i=0;i<max;i++){
		c->inventory[i].ID     = p->v.u16[ii++];
		c->inventory[i].amount = p->v.i16[ii++];
	}
}

void characterSetEquipmentP(character *c, const packet *p){
	if(c == NULL){return;}
	uint max = MIN(CHAR_EQ_MAX,packetLen(p)/4);
	uint ii = 0;
	for(uint i=0;i<max;i++){
		c->equipment[i].ID     = p->v.u16[ii++];
		c->equipment[i].amount = p->v.i16[ii++];
	}
}

float characterGetMaxHookLen(const character *c){
	if(!itemIsEmpty(&c->equipment[CHAR_EQ_HOOK])){
		if(c->equipment[CHAR_EQ_HOOK].ID == I_Hook){return 256.f;}
	}
	return 64;
}

float characterGetHookWinchS(const character *c){
	if(!itemIsEmpty(&c->equipment[CHAR_EQ_HOOK])){
		if(c->equipment[CHAR_EQ_HOOK].ID == I_Hook){return 0.1f;}
	}
	return 0.04f;
}

character *characterGetByBeing(being b){
	const uint i = beingID(b);
	if(beingType(b) != BEING_CHARACTER){ return NULL; }
	if(i >= characterCount)            { return NULL; }
	return &characterList[i];
}

uint characterGetBeing(const character *c){
	if(c == NULL){return 0;}
	return beingCharacter(c - &characterList[0]);
}

character *characterClosest(const vec pos, float maxDistance){
	for(uint i=0;i<characterCount;i++){
		if(characterList[i].hp <= 0){continue;}
		const float d = vecMag(vecSub(pos,characterList[i].pos));
		if(d > maxDistance){continue;}
		return &characterList[i];
	}
	return NULL;
}

void characterAddRecoil(character *c, float recoil){
	const vec vel = vecMulS(vecDegToVec(c->rot),.01f);
	c->vel = vecAdd(c->vel, vecMulS(vel,-0.75f*recoil));
	c->rot = vecAdd(c->rot, vecNew((rngValf()-0.5f) * recoil, (rngValf()-.8f) * recoil, 0.f));
}
