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

void characterSetPos(character *c, float x, float y, float z){
	c->x = x;
	c->y = y;
	c->z = z;
}

void characterSetVelocity(character *c, float vx, float vy, float vz){
	c->vx = vx;
	c->vy = vy;
	c->vz = vz;
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
		return true;
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
	float cvx,cvy,cvz;
	float cx,cy,cz;
	float lx,ly,lz;
	int   lastBlock=-1;
	const float yaw   = c->yaw;
	const float pitch = c->pitch;

	cvx = (cos((yaw-90.f)*PI/180) * cos((-pitch)*PI/180))*0.1f;
	cvy = (sin((-pitch)*PI/180))*0.1f;
	cvz = (sin((yaw-90.f)*PI/180) * cos((-pitch)*PI/180))*0.1f;

	cx = lx = c->x;
	cy = ly = c->y+0.5f;
	cz = lz = c->z;

	for(int i=0;i<50;i++){
		if((lastBlock == -1) || (fabsf(lx - floorf(cx)) > 0.1f) || (fabsf(ly - floorf(cy)) > 0.1f) || (fabsf(lz - floorf(cz)) > 0.1f)){
			lastBlock = worldGetB(cx,cy,cz);
			if(lastBlock > 0){
				if(returnBeforeBlock){
					*retX = lx;
					*retY = ly;
					*retZ = lz;
				}else{
					*retX = cx;
					*retY = cy;
					*retZ = cz;
				}
				return true;
			}
			lx = floorf(cx);
			ly = floorf(cy);
			lz = floorf(cz);
		}

		cx += cvx;
		cy += cvy;
		cz += cvz;
	}
	return false;
}

uint32_t characterCollision(character *c, float cx, float cy, float cz,float wd){
	(void)c;

	uint32_t col = 0;
	const float WD = wd*2.f;

	if(checkCollision(cx-wd,cy+0.5f,cz   )){col |=  0x10;}
	if(checkCollision(cx+wd,cy+0.5f,cz   )){col |=  0x20;}
	if(checkCollision(cx   ,cy+0.5f,cz-wd)){col |=  0x40;}
	if(checkCollision(cx   ,cy+0.5f,cz+wd)){col |=  0x80;}

	if(checkCollision(cx-wd,cy-1.f ,cz   )){col |=   0x1;}
	if(checkCollision(cx+wd,cy-1.f ,cz   )){col |=   0x2;}
	if(checkCollision(cx   ,cy-1.f ,cz-wd)){col |=   0x4;}
	if(checkCollision(cx   ,cy-1.f ,cz+wd)){col |=   0x8;}

	if(checkCollision(cx-WD,cy-0.7f,cz   )){col |= 0x100;}
	if(checkCollision(cx+WD,cy-0.7f,cz   )){col |= 0x200;}
	if(checkCollision(cx   ,cy-0.7f,cz-WD)){col |= 0x400;}
	if(checkCollision(cx   ,cy-0.7f,cz+WD)){col |= 0x800;}

	return col;
}

void characterMove(character *c, float mx,float my,float mz){
	float speed;
	const float yaw   = c->yaw;
	const float pitch = c->pitch;

	if(c->noClip){
		if(c->sneak){
			speed = 1.f;
		}else{
			speed = 0.15f;
		}
		c->gvx = (cos((yaw+90.f)*PI/180) * cos(pitch*PI/180))*mz*speed;
		c->gvy = sin(pitch*PI/180)*mz*speed;
		c->gvz = (sin((yaw+90.f)*PI/180) * cos(pitch*PI/180))*mz*speed;

		c->gvx += cos((yaw)*PI/180)*mx*speed;
		c->gvz += sin((yaw)*PI/180)*mx*speed;
	}else{
		if(c->sneak){
			speed = 0.01f;
		}else{
			speed = 0.05f;
		}
		c->gvy = my;
		c->gvx = (cos((yaw+90)*PI/180)*mz)*speed;
		c->gvz = (sin((yaw+90)*PI/180)*mz)*speed;

		c->gvx += cos((yaw)*PI/180)*mx*speed;
		c->gvz += sin((yaw)*PI/180)*mx*speed;
	}
}

void characterRotate(character *c, float vYaw,float vPitch,float vRoll){
	float yaw   = c->yaw   +   vYaw;
	float pitch = c->pitch + vPitch;
	float roll  = c->roll  +  vRoll;

	if(yaw   > 360.f){yaw   -= 360.f;}
	if(yaw   <   0.f){yaw   += 360.f;}
	if(pitch >  90.f){pitch  =  90.f;}
	if(pitch < -90.f){pitch  = -90.f;}
	if(roll  > 360.f){roll  -= 360.f;}
	if(roll  <   0.f){roll  += 360.f;}

	c->yaw   = yaw;
	c->pitch = pitch;
	c->roll  = roll;
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