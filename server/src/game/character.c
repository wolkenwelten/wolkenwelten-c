#include "character.h"

#include "../main.h"
#include "../voxel/bigchungus.h"
#include "../game/blockType.h"
#include "../game/itemDrop.h"
#include "../game/blockMining.h"
#include "../../../common/src/misc/misc.h"

#include <math.h>
#include <stdio.h>

character characterList[128];
int characterCount = 0;
character *characterFirstFree = NULL;

uint32_t characterCollision(const character *c, float cx, float cy, float cz,float wd){
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

void characterEmptyInventory(character *c){
	for(unsigned int i=0;i<40;i++){
		c->inventory[i] = itemEmpty();
	}
	c->inventory[0] = itemNew(261, 1);
	c->inventory[1] = itemNew(259, 1);
	c->inventory[2] = itemNew(260, 1);
	c->inventory[3] = itemNew(256,99);
	c->inventory[4] = itemNew(257,99);
	c->inventory[5] = itemNew(258,99);
	c->inventory[6] = itemNew(  1,99);
	c->inventory[7] = itemNew(  2,99);
	c->inventory[8] = itemNew(  3,99);
}

void characterInit(character *c){
	int sx,sy,sz;

	c->hitOff = 0.f;
	c->gyoff  = 0.f;
	c->gvx = c->gvy = c->gvz = 0.f;

	c->noClip        = false;
	c->falling       = false;
	c->fallingSound  = false;
	c->sneak         = false;
	c->hasHit        = false;

	c->actionTimeout = 0;
	c->stepTimeout   = 0;
	c->activeItem    = 0;
	c->hitOff        = 0.f;
	c->blockMiningX  = c->blockMiningY = c->blockMiningZ = -1;
	c->hp = c->maxhp = 20;

	bigchungusGetSpawnPos(&world,&sx,&sy,&sz);
	c->yaw   = 135.f;
	c->pitch = 0.f;
	c->roll  = 0.f;
	c->yoff  = 0.f;
	c->x     = sx+0.5f;
	c->y     = sy+1.0f;
	c->z     = sz+0.5f;
	c->vx    = c->vy = c->vz = 0.f;
	c->hook  = false;
	c->hookx = c->hooky = c->hookz = 0.f;

	characterEmptyInventory(c);
}

character *characterNew(){
	character *c = NULL;
	if(characterFirstFree != NULL){
		c = characterFirstFree;
		characterFirstFree = c->nextFree;
	}
	if(c == NULL){
		if(characterCount >= (int)(sizeof(characterList) / sizeof(character))-1){
			fprintf(stderr,"characterList Overflow!\n");
			return NULL;
		}
		c = &characterList[characterCount++];
	}
	c->hook = NULL;
	characterInit(c);

	return c;
}

void characterFree(character *c){
	c->nextFree = characterFirstFree;
	characterFirstFree = c;
}

bool characterLOSBlock(const character *c, int *retX, int *retY, int *retZ, int returnBeforeBlock) {
	float cvx,cvy,cvz;
	float cx,cy,cz;
	float lx,ly,lz;
	int lastBlock=-1;
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

void characterDie(character *c){
	characterInit(c);
}

void characterSetActiveItem(character *c, int i){
	if(i > 9){i = 0;}
	if(i < 0){i = 9;}
	c->activeItem = i;
}

int characterGetItemAmount(const character *c, uint16_t itemID){
	int amount = 0;
	for(unsigned int i=0;i<40;i++){
		if(c->inventory[i].ID == itemID){
			amount += c->inventory[i].amount;
		}
	}
	return amount;
}

bool characterDecItemAmount(character *c, uint16_t itemID,int amount){
	for(unsigned int i=0;i<40;i++){
		if(amount == 0){return true;}
		if(c->inventory[i].ID == itemID){
			if(c->inventory[i].amount > amount){
				itemDecStack(&c->inventory[i],amount);
				return true;
			}else{
				amount -= c->inventory[i].amount;
				c->inventory[i] = itemEmpty();
			}
		}
	}
	if(amount == 0){return true;}
	return false;
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
	
	return (a == amount);
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

int characterGetHP(const character *c){
	return c->hp;
}

int characterGetMaxHP(const character *c){
	return c->maxhp;
}

void characterAddCooldown(character *c, int cooldown){
	c->actionTimeout = -cooldown;
}
