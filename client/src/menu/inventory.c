#include "inventory.h"

#include "../gui/menu.h"
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
int inventoryOpen       = 0;
uint  gamepadSelection  = 4096;
item inventoryCurrentPickup;

widget *inventoryPanel;
widget *equipmentSpace;
widget *inventorySpace;
widget *craftingSpace;
widget *inventoryRadio;
widget *craftingRadio;
widget *craftingInfo;

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
	uint sel = (wid->valItem - player->inventory);
	characterDropItem(player,sel);
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

static void handlerCraftingSlotClick(widget *wid){
	if(wid == NULL){return;}
	int r = wid->valu;
	if(recipeCanCraft(player,r)){
		recipeDoCraft(player,r,1);
	}
}

static void handlerCraftingSlotAltClick(widget *wid){
	if(wid == NULL){return;}
	int r = wid->valu;
	if(recipeCanCraft(player,r)){
		recipeDoCraft(player,r,recipeCanCraft(player,r));
	}
}

static void handlerInventoryRadioInventory(widget *wid){
	(void)wid;
	showInventory();
}

static void handlerInventoryRadioCrafting(widget *wid){
	(void)wid;
	showCrafting();
}

static void handlerCraftingSlotHover(widget *wid){
	craftingInfo->vali = wid->vali;
}

static void handlerCraftingSlotBlur(widget *wid){
	if(craftingInfo->vali == wid->vali){ craftingInfo->vali = -1;}
}

static void initCraftingSpace(){
	const int ts = getTilesize();

	craftingSpace = widgetNewCP(wSpace,inventoryPanel,-1,0, 0,-1);
	craftingRadio = widgetNewCPLH(wRadioButton,inventoryPanel,5*ts,0,5*ts,32,"Crafting","click",handlerInventoryRadioCrafting);
	craftingInfo  = widgetNewCP(wRecipeInfo,craftingSpace,ts/2,32+ts/2,10*ts,ts);
	craftingInfo->vali = 5;
	for(uint r=0;r<MIN(40,recipeGetCount());r++){
		const int x = r%10;
		const int y = (r/10)+2;
		widget *slot = widgetNewCP(wRecipeSlot,craftingSpace,x*ts,y*ts+32,ts,ts);
		slot->vali = r;
		widgetBind(slot,"click",   handlerCraftingSlotClick);
		widgetBind(slot,"altclick",handlerCraftingSlotAltClick);
		widgetBind(slot,"hover",   handlerCraftingSlotHover);
		widgetBind(slot,"blur",    handlerCraftingSlotBlur);
	}
}

static widget *widgetNewItemSlot(widget *parent, int x, int y, item *islot){
	const int ts = getTilesize();

	widget *slot = widgetNewCP(wItemSlot,parent,x,y,ts,ts);
	slot->valItem = islot;
	widgetBind(slot,"click",   handlerInventoryItemClick);
	widgetBind(slot,"altclick",handlerInventoryItemAltClick);
	widgetBind(slot,"midclick",handlerInventoryItemMidClick);
	return slot;
}

static void initInventorySpace(){
	const int ts = getTilesize();

	inventorySpace = widgetNewCP(wSpace,inventoryPanel,-1,0,ts*10,-1);
	inventoryRadio = widgetNewCPLH(wRadioButton,inventoryPanel,0,0,5*ts,32,"Inventory","click",handlerInventoryRadioInventory);
	for(int y=0;y<4;y++){
		for(int x=0;x<10;x++){
			widgetNewItemSlot(inventorySpace,x*ts,y*ts+32,&player->inventory[x+y*10]);
		}
	}
}

static void initEquipmentSpace(){
	const int ts = getTilesize();

	equipmentSpace = widgetNewCP(wSpace,inventoryPanel,-1,0,ts*10,ts*3);
	widgetNewItemSlot(equipmentSpace,5*ts,ts+32,&player->equipment[0]);
	widgetNewItemSlot(equipmentSpace,6*ts+ts/2,ts+32,&player->equipment[1]);
	widgetNewItemSlot(equipmentSpace,8*ts,ts+32,&player->equipment[2]);
	widgetNewCPL(wLabel,equipmentSpace,5*ts-ts/4,32+ts/2,ts,ts/2,"Glider");
	widgetNewCPL(wLabel,equipmentSpace,6*ts+ts/2,32+ts/2,ts,ts/2,"Hook");
	widgetNewCPL(wLabel,equipmentSpace,8*ts,32+ts/2,ts,ts/2,"Pack");
}

void initInventory(){
	const int ts = getTilesize();

	if(inventoryPanel != NULL){ widgetFree(inventoryPanel); }
	inventoryPanel = widgetNewCP(wPanel,rootMenu,-1,-1,ts*10,0);
	inventoryPanel->flags |= WIDGET_HIDDEN;

	initInventorySpace();
	initEquipmentSpace();
	initCraftingSpace();
	inventoryRadio->flags |= WIDGET_ACTIVE;

	switch(inventoryOpen){
	default:
	case 0:
		showInventoryPanel();
		break;
	case 1:
		showInventory();
		break;
	case 2:
		showCrafting();
		break;
	}
}

void showInventory(){
	const int gh = getTilesize()* 7 + 32;
	if(!gameRunning){return;}

	if((inventoryPanel->h == gh) && (inventoryRadio->flags & WIDGET_ACTIVE)){
		hideInventory();
		return;
	}
	inventoryOpen = 1;
	showMouseCursor();
	widgetSlideH(inventoryPanel,              gh);
	widgetSlideW(inventorySpace,getTilesize()*10);
	widgetSlideY(inventorySpace,getTilesize()* 3);
	widgetSlideW(equipmentSpace,getTilesize()*10);
	widgetSlideW(craftingSpace,                0);
	inventoryRadio->flags |= WIDGET_ACTIVE;
	craftingRadio->flags &= ~WIDGET_ACTIVE;
	widgetFocus(NULL);
}
void showCrafting(){
	int gh = getTilesize() * ((recipeGetCount()/10)+2) + 32;
	if(recipeGetCount() % 10){gh+=getTilesize();}
	if(!gameRunning){return;}
	if((inventoryPanel->h == gh) && (craftingRadio->flags & WIDGET_ACTIVE)){
		hideInventory();
		return;
	}
	inventoryOpen = 2;
	showMouseCursor();
	widgetSlideH(inventoryPanel,              gh);
	widgetSlideW(inventorySpace,               0);
	widgetSlideW(equipmentSpace,               0);
	widgetSlideW(craftingSpace, getTilesize()*10);
	craftingRadio->flags |= WIDGET_ACTIVE;
	inventoryRadio->flags &= ~WIDGET_ACTIVE;
	widgetFocus(NULL);
}

void hideInventory(){
	if(!gameRunning){return;}
	inventoryOpen = false;
	if(!itemIsEmpty(&inventoryCurrentPickup)){
		itemDropNewC(player, &inventoryCurrentPickup);
		itemDiscard(&inventoryCurrentPickup);
	}
	showInventoryPanel();
	widgetFocus(widgetGameScreen);
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

void showInventoryPanel(){
	widgetSlideH(inventoryPanel,getTilesize()+32);
	widgetSlideW(inventorySpace,getTilesize()*10);
	widgetSlideY(inventorySpace,               0);
	widgetSlideW(equipmentSpace,               0);
	widgetSlideW(craftingSpace,                0);
}

void hideInventoryPanel(){
	widgetSlideH(inventoryPanel,               0);
	widgetSlideW(inventorySpace,getTilesize()*10);
	widgetSlideW(craftingSpace,                0);
	widgetSlideW(equipmentSpace,               0);
}
