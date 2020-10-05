#include "inventory.h"

#include "../game/character.h"
#include "../game/itemDrop.h"
#include "../game/recipe.h"
#include "../../../common/src/game/item.h"
#include "../gfx/gfx.h"
#include "../gfx/textMesh.h"
#include "../gui/gui.h"
#include "../sdl/sfx.h"
#include "../main.h"

#include <stddef.h>
#include <math.h>

const float ITEMTILE    = (1.f/32.f);
bool inventoryOpen      = false;
uint  gamepadSelection  = 4096;
item inventoryCurrentPickup;

widget *inventoryPanel;

static void handlerInventoryItemClick(widget *wid){
	item *cItem = wid->valItem;
	if(cItem == NULL){return;}

	if(itemIsEmpty(&inventoryCurrentPickup)){
		if(itemIsEmpty(cItem)){return;}
		inventoryCurrentPickup = *cItem;
		itemDiscard(cItem);
	}else{
		if(cItem == NULL){return;}
		if(itemCanStack(cItem,inventoryCurrentPickup.ID) || (cItem->ID == inventoryCurrentPickup.ID)){
			inventoryCurrentPickup.amount -= itemIncStack(cItem,inventoryCurrentPickup.amount);
		}else{
			item tmp = inventoryCurrentPickup;
			inventoryCurrentPickup = *cItem;
			*cItem = tmp;
		}
	}
	sfxPlay(sfxPock,1.f);
}

static void handlerInventoryItemMidClick(widget *wid){
	item *cItem = wid->valItem;
	if(cItem == NULL){return;}
	if(itemIsEmpty(cItem)){return;}
	//characterDropItem(player,sel);
	sfxPlay(sfxPock,1.f);
}

static void handlerInventoryItemAltClick(widget *wid){
	item *cItem = wid->valItem;
	if(cItem == NULL){return;}

	if(itemIsEmpty(&inventoryCurrentPickup)){
		if(itemIsEmpty(cItem)){return;}
		inventoryCurrentPickup = *cItem;
		cItem->amount /= 2;
		inventoryCurrentPickup.amount -= cItem->amount;
	}else{
		if(cItem == NULL){return;}
		if(!itemCanStack(cItem,inventoryCurrentPickup.ID) && !itemIsEmpty(cItem)){return;}
		cItem->ID = inventoryCurrentPickup.ID;
		itemIncStack(cItem,1);
		itemDecStack(&inventoryCurrentPickup,1);
	}
	sfxPlay(sfxPock,1.f);
}

void initInventory(){
	const int ts = getTilesize();
	const int sx = 10*ts;
	inventoryPanel = widgetNewCP(wPanel,rootHud,-1,-1,sx,ts);
	for(int y=0;y<4;y++){
		for(int x=0;x<10;x++){
			widget *slot = widgetNewCP(wItemSlot,inventoryPanel,x*ts,y*ts,ts,ts);
			slot->valItem = &player->inventory[x+y*10];
			widgetBind(slot,"click",handlerInventoryItemClick);
			widgetBind(slot,"altclick",handlerInventoryItemAltClick);
			widgetBind(slot,"midclick",handlerInventoryItemMidClick);
		}
	}
}

void showInventory(){
	if(!gameRunning){return;}
	inventoryOpen = true;
	showMouseCursor();
	widgetSlideH(inventoryPanel,getTilesize()*4);
}

void hideInventory(){
	if(!gameRunning){return;}
	inventoryOpen = false;
	hideMouseCursor();
	if(!itemIsEmpty(&inventoryCurrentPickup)){
		itemDropNewC(player, &inventoryCurrentPickup);
		itemDiscard(&inventoryCurrentPickup);
	}
	widgetSlideH(inventoryPanel,getTilesize());
}

bool isInventoryOpen(){
	return inventoryOpen;
}

void drawInventory(textMesh *guim){
	static uint ticks = 0;
	const uint tilesize = getTilesize();
	if(!isInventoryOpen()){return;}

	const int animX = sin((float)ticks/24.f)*tilesize/8;
	const int animY = cos((float)ticks/24.f)*tilesize/8;
	++ticks;

	if(!mouseHidden){
		textMeshItem(guim,mousex+animX-tilesize/8,mousey+animY-tilesize/8,tilesize,3,&inventoryCurrentPickup);
	}
}
