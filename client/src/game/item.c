#include "item.h"
#include "../mods/mods.h"
#include "../game/blockType.h"
#include "../game/character.h"
#include "../game/grenade.h"
#include "../gfx/objs.h"
#include "../sdl/sfx.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/network/messages.h"

item itemNew(uint16_t ID, int16_t amount){
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
	i->amount = i->ID = 0;
}

bool itemIsEmpty(item *i){
	return ((i == NULL) || (i->amount==0) || (i->ID==0));
}

int itemBlockDamage(item *i, blockCategory cat){
	return blockDamageDispatch(i,cat);
}

mesh *itemGetMesh(item *i){
	return getMeshDispatch(i);
}
float itemGetInaccuracy(item *i){
	return getInaccuracyDispatch(i);
}
bool itemIsSingle(item *i){
	return (getStackSizeDispatch(i) == 1);
}
bool itemHasPrimaryAction(item *i){
	return hasPrimaryAction(i);
}
bool itemPrimaryAction(item *i, character *chr, int to){
	return primaryActionDispatch(i,chr,to);
}
bool itemSecondaryAction(item *i, character *chr, int to){
	return secondaryActionDispatch(i,chr,to);
}
bool itemTertiaryAction(item *i, character *chr, int to){
	return tertiaryActionDispatch(i,chr,to);
}

int itemCanStack(item *i, uint16_t ID){
	const int ma = getStackSizeDispatch(i);
	if(itemIsSingle(i)){return 0;}
	if(i->ID != ID)    {return 0;}
	if(i->amount >= ma){return 0;}
	if(i->amount ==  0){return 0;}
	return ma-i->amount;
}
int itemIncStack(item *i, int16_t amount){
	const int ma = getStackSizeDispatch(i);
	if((i->amount+amount)>ma){amount = ma - i->amount;}
	i->amount += amount;
	return amount;
}
int itemDecStack(item *i, int16_t amount){
	if(i->amount < amount){amount = i->amount;}
	i->amount -= amount;
	return amount;
}

bool itemPlaceBlock(item *i, character *chr, int to){
	int cx,cy,cz;
	if(to < 0){return false;}
	if(characterLOSBlock(chr,&cx,&cy,&cz,true)){
		if((characterCollision(chr,chr->x,chr->y,chr->z,0.3f)&0xFF0)){ return false; }
		if(!itemDecStack(i,1)){ return false; }
		worldSetB(cx,cy,cz,i->ID);
		if((characterCollision(chr,chr->x,chr->y,chr->z,0.3f)&0xFF0) != 0){
			worldSetB(cx,cy,cz,0);
			itemIncStack(i,1);
			return false;
		} else {
			msgPlaceBlock(cx,cy,cz,i->ID);
			sfxPlay(sfxPock,1.f);
			characterAddCooldown(chr,50);
			return true;
		}
	}
	return false;
}
