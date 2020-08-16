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
	i.amount = MIN(99,amount);
	i.ID     = ID;
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
	return ((i->amount==0) || (i->ID==0));
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
	return isSingleItemDispatch(i);
}

bool itemHasMineAction(item *i){
	return hasMineActionDispatch(i);
}

bool itemMineAction(item *i, character *chr, int to){
	return mineActionDispatch(i,chr,to);
}

int itemCanStack(item *i, uint16_t ID){
	if(itemIsSingle(i)){return 0;}
	if(i->ID != ID)    {return 0;}
	if(i->amount >= 99){return 0;}
	if(i->amount ==  0){return 0;}
	return 99-i->amount;
}
int itemIncStack(item *i, int16_t amount){
	if((i->amount+amount)>99){amount = 99 - i->amount;}
	i->amount += amount;
	return amount;
}
int itemDecStack(item *i, int16_t amount){
	if(i->amount < amount){amount = i->amount;}
	i->amount -= amount;
	return amount;
}

bool itemActivateBlock(item *i, character *chr, int to){
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

bool itemActivate(item *i, character *chr, int to){
	if(i->ID < 256){
		return itemActivateBlock(i,chr, to);
	}
	return activateItemDispatch(i,chr,to);
}
