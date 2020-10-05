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
	inventoryPanel = widgetNewCP(WIDGET_PANEL,rootHud,-1,-1,sx,ts);
	for(int y=0;y<4;y++){
		for(int x=0;x<10;x++){
			widget *slot = widgetNewCP(WIDGET_ITEMSLOT,inventoryPanel,x*ts,y*ts,ts,ts);
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

void inventoryGamepadSelToXY(int sel,int *rx, int *ry){
	int xsel = (sel % 10)-5;
	int ysel = 5 - (sel / 10);

	int tilesize;
	if(screenWidth < 1024){
		tilesize = 48;
	}else if(screenWidth < 1536){
		tilesize = 64;
	}else {
		tilesize = 80;
	}

	*rx = (xsel * tilesize) + screenWidth/2 + tilesize/4;
	*ry = (ysel * tilesize) + (screenHeight/2-2*tilesize) + tilesize/4;
	if(ysel == 0){
		*ry -= tilesize;
	}else if(ysel == 5){
		*ry += tilesize/2;
	}
}

void drawInventory(textMesh *guim){
	static uint ticks = 0;
	const uint tilesize = getTilesize();
	if(!isInventoryOpen()){return;}

	const int animX = sin((float)ticks/24.f)*tilesize/8;
	const int animY = cos((float)ticks/24.f)*tilesize/8;
	++ticks;

	if(mouseHidden){
		int gx,gy;
		inventoryGamepadSelToXY(gamepadSelection,&gx,&gy);
		textMeshItem(guim,gx+animX,gy+animY,tilesize,3,&inventoryCurrentPickup);
	}else{
		textMeshItem(guim,mousex+animX-tilesize/8,mousey+animY-tilesize/8,tilesize,3,&inventoryCurrentPickup);
	}
}

void inventoryClickOutside(int btn){
	return;
	if((btn == 1) && !itemIsEmpty(&inventoryCurrentPickup)){
		itemDropNewC(player, &inventoryCurrentPickup);
		itemDiscard(&inventoryCurrentPickup);
	}
}

void doInventoryClick(int btn, uint sel){
	item *cItem = NULL;
	return;

	if((sel >= 50) && (sel < (50+MIN(10,recipeGetCraftableCount(player))))){
		int r = recipeGetCraftableIndex(player,sel-50);
		if(btn == 1){
			recipeDoCraft(player,r,1);
		}else if(btn == 3){
			recipeDoCraft(player,r,recipeCanCraft(player,r));
		}
		return;
	}

	if(btn == 1){
		if(itemIsEmpty(&inventoryCurrentPickup)){
			cItem = characterGetItemBarSlot(player,sel);
			if(itemIsEmpty(cItem)){return;}
			inventoryCurrentPickup = *cItem;
			itemDiscard(cItem);
		}else{
			cItem = characterGetItemBarSlot(player,sel);
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
	if(btn == 2){
		cItem = characterGetItemBarSlot(player,sel);
		if(itemIsEmpty(cItem)){return;}
		characterDropItem(player,sel);
		sfxPlay(sfxPock,1.f);
	}
	if(btn == 3){
		if(itemIsEmpty(&inventoryCurrentPickup)){
			cItem = characterGetItemBarSlot(player,sel);
			if(itemIsEmpty(cItem)){return;}
			inventoryCurrentPickup = *cItem;
			cItem->amount /= 2;
			inventoryCurrentPickup.amount -= cItem->amount;
		}else{
			cItem = characterGetItemBarSlot(player,sel);
			if(cItem == NULL){return;}
			if(!itemCanStack(cItem,inventoryCurrentPickup.ID) && !itemIsEmpty(cItem)){return;}
			cItem->ID = inventoryCurrentPickup.ID;
			itemIncStack(cItem,1);
			itemDecStack(&inventoryCurrentPickup,1);
		}
		sfxPlay(sfxPock,1.f);
	}
}

void updateInventoryClick(int x,int y, int btn){
	int tilesize;
	if(!isInventoryOpen()){return;}

	if(screenWidth < 1024){
		tilesize = 48;
	}else if(screenWidth < 1536){
		tilesize = 64;
	}else {
		tilesize = 80;
	}
	if(x < ((screenWidth /2)-(5*tilesize)))           {inventoryClickOutside(btn);return;}
	if(x > ((screenWidth /2)+(5*tilesize)))           {inventoryClickOutside(btn);return;}
	if(y < ((screenHeight/2)-(3*tilesize)))           {inventoryClickOutside(btn);return;}
	if(y > ((screenHeight/2)+(4*tilesize)+tilesize/2)){inventoryClickOutside(btn);return;}

	int xsel = (x - (screenWidth/2-5*tilesize))/tilesize;
	int ysel = 4-(((y+tilesize) - (screenHeight/2))/tilesize);
	if(ysel == 0){ysel = -1;}
	if(y > ((screenHeight/2)+tilesize*4-tilesize/2)){
		ysel = 4-(((y+tilesize/2) - (screenHeight/2))/tilesize);
		if(ysel == 1){ inventoryClickOutside(btn);return;}
	}
	int sel = xsel + ysel*10;
	if(ysel == 4){return;}
	doInventoryClick(btn,sel);
}

void changeInventorySelection(int x,int y){
	mouseHidden = true;
	if(gamepadSelection > 2048){
		gamepadSelection = 0;
		return;
	}
	int xsel = gamepadSelection % 10;
	int ysel = gamepadSelection / 10;

	ysel += y;
	if(ysel < 0){ysel = 5;}
	if(ysel == 4){
		if(y > 0){
			ysel = 5;
		}else{
			ysel = 3;
		}
	}
	if(ysel > 5){ysel = 0;}
	int xmax = 9;
	if(ysel == 5){xmax = MIN(9,recipeGetCraftableCount(player)-1);}
	xsel += x;
	if(xsel < 0){xsel = xmax;}
	if(xsel > xmax){xsel = 0;}
	gamepadSelection = xsel + ysel*10;
}

void updateInventoryGamepad(int btn){
	mouseHidden = true;
	if(gamepadSelection > 2048){
		gamepadSelection = 0;
		return;
	}
	if(btn == 2){
		btn = 3;
	}else if(btn == 3){
		btn = 2;
	}
	doInventoryClick(btn,gamepadSelection);
}
