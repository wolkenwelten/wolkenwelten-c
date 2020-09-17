#include "character.h"
#include "item.h"
#include "../mods/api_v1.h"
#include "../mods/mods.h"

#include <math.h>

int characterGetHP(character *c){
	return c->hp;
}

int characterGetMaxHP(character *c){
	return c->maxhp;
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

void characterAddInaccuracy(character *c, float inc){
	c->inaccuracy += inc;
}

int characterGetItemAmount(character *c, uint16_t itemID){
	int amount = 0;
	for(unsigned int i=0;i<40;i++){
		if(c->inventory[i].ID == itemID){
			amount += c->inventory[i].amount;
		}
	}
	return amount;
}

int characterDecItemAmount(character *c, uint16_t itemID,int amount){
	int ret=0;

	if(amount == 0){return 0;}
	for(unsigned int i=0;i<40;i++){
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

bool characterPickupItem(character *c, uint16_t itemID,int amount){
	int a = 0;
	item ci = itemNew(itemID,amount);
	if(getStackSizeDispatch(&ci) == 1){
		ci.amount = amount;
		for(unsigned int i=0;i<40;i++){
			if(itemIsEmpty(&c->inventory[i])){
				c->inventory[i] = ci;
				return true;
			}
		}
		return false;
	}

	for(unsigned int i=0;i<40;i++){
		if(a >= amount){break;}
		if(itemCanStack(&c->inventory[i],itemID)){
			a += itemIncStack(&c->inventory[i],amount - a);
		}
	}
	for(unsigned int i=0;i<40;i++){
		if(a >= amount){break;}
		if(itemIsEmpty(&c->inventory[i])){
			c->inventory[i] = itemNew(itemID,amount - a);
			a += c->inventory[i].amount;
		}
	}

	if(a == amount){
		sfxPlay(sfxPock,.8f);
		return true;
	}
	return false;
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

bool characterDamage(character *c, int hp){
	return characterHP(c,-hp);
}

void characterEmptyInventory(character *c){
	for(unsigned int i=0;i<40;i++){
		c->inventory[i] = itemEmpty();
	}
}

item *characterGetItemBarSlot(character *c, int i){
	if(i <  0){return NULL;}
	if(i > 40){return NULL;}
	return &c->inventory[i];
}

void characterSetItemBarSlot(character *c, int i, item *itm){
	if(i <  0){return;}
	if(i > 40){return;}
	c->inventory[i] = *itm;
}

void characterSetActiveItem(character *c, int i){
	if(i > 9){i = 0;}
	if(i < 0){i = 9;}
	if((unsigned int)i != c->activeItem){
		characterStartAnimation(c,5,200);
	}
	c->activeItem = i;
}

void characterSwapItemSlots(character *c, int a,int b){
	if(a <  0){return;}
	if(b <  0){return;}
	if(a > 39){return;}
	if(b > 39){return;}

	item tmp = c->inventory[a];
	c->inventory[a] = c->inventory[b];
	c->inventory[b] = tmp;
}

bool characterLOSBlock(character *c, int *retX, int *retY, int *retZ, int returnBeforeBlock) {
	int   lastBlock=-1;

	const vec cv = vecDegToVec(c->rot);
	vec       c  = vecAdd(c->pos,vecNew(0,0.5,0));
	vec       l  = c;

	for(int i=0;i<50;i++){
		if((lastBlock == -1) || (fabsf(lx - floorf(cx)) > 0.1f) || (fabsf(ly - floorf(cy)) > 0.1f) || (fabsf(lz - floorf(cz)) > 0.1f)){
			lastBlock = worldGetB(c.x,c.y,c.z);
			if(lastBlock > 0){
				if(returnBeforeBlock){
					*retX = l.x;
					*retY = l.y;
					*retZ = l.z;
				}else{
					*retX = c.x;
					*retY = c.y;
					*retZ = c.z;
				}
				return true;
			}
			l = vecFloor(c);
		}
		c = vecAdd(c,cv);
	}
	return false;
}

uint32_t characterCollision(const vec c){
	uint32_t col = 0;
	const float wd = 0.2f;
	const float WD = wd*2.f;

	if(checkCollision(c.x-wd,c.y+0.5f,c.z   )){col |=  0x10;}
	if(checkCollision(c.x+wd,c.y+0.5f,c.z   )){col |=  0x20;}
	if(checkCollision(c.x   ,c.y+0.5f,c.z-wd)){col |=  0x40;}
	if(checkCollision(c.x   ,c.y+0.5f,c.z+wd)){col |=  0x80;}

	if(checkCollision(c.x-wd,c.y-1.f ,c.z   )){col |=   0x1;}
	if(checkCollision(c.x+wd,c.y-1.f ,c.z   )){col |=   0x2;}
	if(checkCollision(c.x   ,c.y-1.f ,c.z-wd)){col |=   0x4;}
	if(checkCollision(c.x   ,c.y-1.f ,c.z+wd)){col |=   0x8;}

	if(checkCollision(c.x-WD,c.y-0.7f,c.z   )){col |= 0x100;}
	if(checkCollision(c.x+WD,c.y-0.7f,c.z   )){col |= 0x200;}
	if(checkCollision(c.x   ,c.y-0.7f,c.z-WD)){col |= 0x400;}
	if(checkCollision(c.x   ,c.y-0.7f,c.z+WD)){col |= 0x800;}

	return col;
}

void characterMove(character *c, const vec mov){
	const float yaw   = c->rot.yaw;
	const float pitch = c->rot.pitch;

	if(c->flags & CHAR_NOCLIP){
		c->gvel = vecMulS(vecDegToVec(c->rot),mov.z)
		c->gvz = (sin((yaw+90.f)*PI/180) * cos(pitch*PI/180))*mov.z;

		c->gvel.x += cos((yaw)*PI/180)*mov.x;
		c->gvel.z += sin((yaw)*PI/180)*mov.x;
	}else{
		c->gvel.y = mov.y;
		c->gvel.x = (cos((yaw+90)*PI/180)*mov.z);
		c->gvel.z = (sin((yaw+90)*PI/180)*mov.z);

		c->gvel.x += cos((yaw)*PI/180)*mov.x;
		c->gvel.z += sin((yaw)*PI/180)*mov.x;
	}
}

void characterRotate(character *c, const vec rot){
	c->rot = vecAdd(c->rot,rot);
}

void characterStartAnimation(character *c, int index, int duration){
	c->animationIndex = index;
	c->animationTicksLeft = c->animationTicksMax = duration;
}

bool characterItemReload(character *c, item *i, int cooldown){
	const int MAGSIZE = getMagSizeDispatch(i);
	const int AMMO    = getAmmunitionDispatch(i);
	int ammoleft      = characterGetItemAmount(c,AMMO);

	if(c->actionTimeout < 0)      {return false;}
	if(itemGetAmmo(i) == MAGSIZE) {return false;}
	if(ammoleft <= 0)             {return false;}

	ammoleft = MIN(MAGSIZE,ammoleft);
	characterDecItemAmount(c, AMMO, itemIncAmmo(i,ammoleft));

	characterAddCooldown(c,cooldown);
	sfxPlay(sfxHookReturned,1.f);
	characterStartAnimation(c,2,MAX(cooldown,400));

	return true;
}

bool characterTryToShoot(character *c, item *i, int cooldown, int bulletcount){
	if(c->actionTimeout < 0){return false;}
	if(itemGetAmmo(i) < bulletcount){
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

void characterSetInventoryP(character *c, const packet *p){
	if(c == NULL){return;}
	int max = MIN(40,packetLen(p)/4);
	for(int i=0;i<max;i++){
		c->inventory[i].ID     = p->val.s[(i<<1)  ];
		c->inventory[i].amount = p->val.s[(i<<1)+1];
	}
}
