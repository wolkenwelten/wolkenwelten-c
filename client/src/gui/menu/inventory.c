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
#include "inventory.h"

#include "../menu.h"
#include "../gui.h"
#include "../../main.h"
#include "../../game/character/character.h"
#include "../../game/itemDrop.h"
#include "../../game/recipe.h"
#include "../../gfx/gfx.h"
#include "../../gfx/textMesh.h"
#include "../../sdl/sdl.h"
#include "../../sfx/sfx.h"
#include "../../../../common/src/game/item.h"

#include <stddef.h>
#include <math.h>

int  inventoryGuiSize  = 0;
int  inventoryOpen     = 0;
uint gamepadSelection  = 4096;
item inventoryCurrentPickup;

widget *inventoryPanel;
widget *equipmentSpace;
widget *inventorySpace;
widget *craftingSpace;
widget *inventoryRadio;
widget *craftingRadio;
widget *craftingInfo;

static void handlerInventoryItemClick(widget *wid){
	item *cItem = wid->valItemSlot;
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
	item *cItem = wid->valItemSlot;
	if(cItem == NULL){return;}
	if(itemIsEmpty(cItem)){return;}
	uint sel = (wid->valItemSlot - player->inventory);
	characterItemDrop(player,sel);
	sfxPlay(sfxPock,1.f);
}

static void handlerInventoryItemAltClick(widget *wid){
	item *cItem = wid->valItemSlot;
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
	openInventory();
}

static void handlerInventoryRadioCrafting(widget *wid){
	(void)wid;
	openCrafting();
}

static void handlerCraftingSlotHover(widget *wid){
	craftingInfo->vali = wid->vali;
}

static void handlerCraftingSlotBlur(widget *wid){
	if(craftingInfo->vali == wid->vali){ craftingInfo->vali = -1;}
}

static void initCraftingSpace(){
	const int ts = getTilesize();

	craftingSpace = widgetNewCP(wSpace,inventoryPanel,rect(-1,32, 0,-33));
	craftingInfo  = widgetNewCP(wRecipeInfo,craftingSpace,rect(ts/2,32+ts/2,10*ts,ts));
	craftingInfo->vali = 5;
	for(uint r=0;r<MIN(40,recipeGetCount());r++){
		const int x = r%10;
		const int y = (r/10)+3;
		widget *slot = widgetNewCP(wRecipeSlot,craftingSpace,rect(x*ts,y*ts+32,ts,ts));
		slot->vali = r;
		widgetBind(slot,"click",   handlerCraftingSlotClick);
		widgetBind(slot,"altclick",handlerCraftingSlotAltClick);
		widgetBind(slot,"hover",   handlerCraftingSlotHover);
		widgetBind(slot,"blur",    handlerCraftingSlotBlur);
	}
}

static widget *widgetNewItemSlot(widget *parent, int x, int y, item *iSlot){
	const int ts = getTilesize();

	widget *slot = widgetNewCP(wItemSlot,parent,rect(x,y,ts,ts));
	slot->valItemSlot = iSlot;
	widgetBind(slot,"click",   handlerInventoryItemClick);
	widgetBind(slot,"altclick",handlerInventoryItemAltClick);
	widgetBind(slot,"midclick",handlerInventoryItemMidClick);
	return slot;
}

static void handlerEquipmentItemClick(widget *wid){
	item *cItem = wid->valItemSlot;
	if(cItem == NULL){return;}

	if(itemIsEmpty(&inventoryCurrentPickup)){
		if(itemIsEmpty(cItem)){return;}
		inventoryCurrentPickup = *cItem;
		itemDiscard(cItem);
	}else{
		const int i = cItem - player->equipment;
		if(!itemIsValidEquipment(&inventoryCurrentPickup,i)){
			sfxPlay(sfxTock,1.f);
			return;
		}
		item tmp = inventoryCurrentPickup;
		inventoryCurrentPickup = *cItem;
		*cItem = tmp;
	}
	sfxPlay(sfxPock,1.f);
	characterUpdateEquipment(player);
	openInventory();
}

static widget *widgetNewEquipmentSlot(widget *parent, int x, int y, item *iSlot){
	const int ts = getTilesize();

	widget *slot = widgetNewCP(wItemSlot,parent,rect(x,y,ts,ts));
	slot->valItemSlot = iSlot;
	widgetBind(slot,"click",handlerEquipmentItemClick);
	return slot;
}

static void refreshInventorySpace(){
	const int ts = getTilesize();

	widgetEmpty(inventorySpace);
	if(player == NULL){return;}
	int i = 0;
	for(int y=0;y<(player->inventorySize/10);y++){
		for(int x=0;x<10;x++){
			if(i > player->inventorySize){break;}
			widgetNewItemSlot(inventorySpace,x*ts,y*ts+32,&player->inventory[i++]);
		}
	}
	inventoryGuiSize = player->inventorySize;
}

static void initInventorySpace(){
	const int ts = getTilesize();

	inventorySpace = widgetNewCP(wSpace,inventoryPanel,rect(-1,32,ts*10,-33));
	refreshInventorySpace();
}

static void initEquipmentSpace(){
	const int ts = getTilesize();

	equipmentSpace = widgetNewCP(wSpace,inventoryPanel,rect(-1,32,ts*10,ts*3));
	widgetNewEquipmentSlot(equipmentSpace,5*ts,ts+32,&player->equipment[0]);
	widgetNewEquipmentSlot(equipmentSpace,6*ts+ts/2,ts+32,&player->equipment[1]);
	widgetNewEquipmentSlot(equipmentSpace,8*ts,ts+32,&player->equipment[2]);
	widgetNewCPL(wLabel,equipmentSpace,rect(5*ts-ts/4,32+ts/2,ts,ts/2),"Glider");
	widgetNewCPL(wLabel,equipmentSpace,rect(6*ts+ts/2,32+ts/2,ts,ts/2),"Hook");
	widgetNewCPL(wLabel,equipmentSpace,rect(8*ts,32+ts/2,ts,ts/2),"Pack");
}

void initInventory(){
	const int ts = getTilesize();

	if(inventoryPanel != NULL){
		widgetFree(inventoryPanel);
		inventorySpace = NULL;
	}
	inventoryPanel = widgetNewCP(wPanel,rootMenu,rect(-1,-1,ts*10,0));
	inventoryPanel->flags |= WIDGET_HIDDEN;

	initInventorySpace();
	initEquipmentSpace();
	initCraftingSpace();
	inventoryRadio = widgetNewCPLH(wRadioButton,inventoryPanel,rect(0,0,5*ts,32),"Inventory","click",handlerInventoryRadioInventory);
	craftingRadio  = widgetNewCPLH(wRadioButton,inventoryPanel,rect(5*ts,0,5*ts,32),"Crafting","click",handlerInventoryRadioCrafting);
	inventoryRadio->flags |= WIDGET_ACTIVE;

	if(!gameRunning){return;}
	switch(inventoryOpen){
	default:
	case 0:
		openInventoryPanel();
		break;
	case 1:
		openInventory();
		break;
	case 2:
		openCrafting();
		break;
	}
}

void openInventory(){
	const int invSize = player != NULL ? player->inventorySize : 20;
	const int gh = (getTilesize() * (3 + (invSize / 10))) + 32;
	if(!gameRunning){return;}
	if(inventoryGuiSize != invSize){refreshInventorySpace();}

	if((inventoryPanel->area.h == gh) && (inventoryRadio->flags & WIDGET_ACTIVE)){
		closeInventory();
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
void openCrafting(){
	int gh = getTilesize() * (1+((recipeGetCount()/10)+2)) + 32;
	if(recipeGetCount() % 10){gh+=getTilesize();}
	if(!gameRunning){return;}
	if((inventoryPanel->area.h == gh) && (craftingRadio->flags & WIDGET_ACTIVE)){
		closeInventory();
		return;
	}
	inventoryOpen = 2;
	hideMouseCursor();
	widgetSlideH(inventoryPanel,              gh);
	widgetSlideW(inventorySpace,               0);
	widgetSlideW(equipmentSpace,               0);
	widgetSlideW(craftingSpace, getTilesize()*10);
	craftingRadio->flags |= WIDGET_ACTIVE;
	inventoryRadio->flags &= ~WIDGET_ACTIVE;
	widgetFocus(NULL);
}

void closeInventory(){
	if(!gameRunning){return;}
	inventoryOpen = false;
	openInventoryPanel();
	widgetFocus(widgetGameScreen);
}
void toggleInventory(){
	if(inventoryOpen){
		closeInventory();
	}else{
		openInventory();
	}
}

bool isInventoryOpen(){
	return inventoryOpen;
}

void drawInventory(textMesh *guim){
	static uint ticks = 0;
	static uint lastDrop = 0;
	const uint tilesize = getTilesize();
	if(!isInventoryOpen()){return;}

	const int animX = sinf((float)ticks/24.f)*tilesize/8;
	const int animY = cosf((float)ticks/24.f)*tilesize/8;
	++ticks;

	if(!mouseHidden && !itemIsEmpty(&inventoryCurrentPickup)){
		textMeshItem(guim,mousex+animX-tilesize/8,mousey+animY-tilesize/8,tilesize,3,&inventoryCurrentPickup);
		if(((int)mousex > (screenWidth  - inventoryPanel->area.w)) &&
		   ((int)mousey > (screenHeight - inventoryPanel->area.h))) {return;}

		if(mouseClicked[0]){
			itemDropNewC(player, &inventoryCurrentPickup);
			itemDiscard(&inventoryCurrentPickup);
		}else if(mouseClicked[2]){
			uint curTicks = getTicks();
			if(curTicks < lastDrop + 50){
				return;
			}
			lastDrop     = curTicks;
			item dItem   = itemNew(inventoryCurrentPickup.ID,1);
			dItem.amount = itemDecStack(&inventoryCurrentPickup,1);
			itemDropNewC(player,&dItem);
		}
	}
}

void openInventoryPanel(){
	widgetSlideH(inventoryPanel,getTilesize()+32);
	widgetSlideW(inventorySpace,getTilesize()*10);
	widgetSlideY(inventorySpace,               0);
	widgetSlideW(equipmentSpace,               0);
	widgetSlideW(craftingSpace,                0);
	inventoryRadio->flags |= WIDGET_ACTIVE;
	craftingRadio->flags &= ~WIDGET_ACTIVE;
}

void closeInventoryPanel(){
	widgetSlideH(inventoryPanel,               0);
	widgetSlideW(inventorySpace,getTilesize()*10);
	widgetSlideW(craftingSpace,                0);
	widgetSlideW(equipmentSpace,               0);
}

void inventoryCheckCursorItem(){
	if(isInventoryOpen()){return;}
	if(!itemIsEmpty(&inventoryCurrentPickup)){
		itemDropNewC(player, &inventoryCurrentPickup);
		itemDiscard(&inventoryCurrentPickup);
	}
}
