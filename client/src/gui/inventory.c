#include "inventory.h"
#include "../main.h"
#include "../gfx/mesh.h"
#include "../game/character.h"
#include "../game/item.h"
#include "../game/itemDrop.h"
#include "../gfx/gfx.h"
#include "../sdl/sdl.h"
#include "../gui/gui.h"
#include "../sdl/sfx.h"
#include "../game/recipe.h"

#include <math.h>

const float ITEMTILE    = (1.f/32.f);
bool inventoryOpen      = false;
int  gamepadSelection   = -1;
item inventoryCurrentPickup;


void showInventory(){
	inventoryOpen = true;
	showMouseCursor();
}

void hideInventory(){
	inventoryOpen = false;
	hideMouseCursor();
	if(!itemIsEmpty(&inventoryCurrentPickup)){
		itemDropNewC(player, &inventoryCurrentPickup);
		itemDiscard(&inventoryCurrentPickup);
	}
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
	static int ticks = 0;
	int tilesize;
	if(!isInventoryOpen()){return;}
	if(screenWidth < 1024){
		tilesize = 48;
	}else if(screenWidth < 1536){
		tilesize = 64;
	}else {
		tilesize = 80;
	}
	const int animX = sin((float)ticks/24.f)*tilesize/8;
	const int animY = cos((float)ticks/24.f)*tilesize/8;
	++ticks;
	guim->size = 2;

	int xraw = (mousex - (screenWidth/2-5*tilesize));
	int xsel = xraw/tilesize;
	int ysel = 4-(((mousey+tilesize) - (screenHeight/2))/tilesize);
	if(ysel == 0){ysel = -1;}
	if(mousey > ((screenHeight/2)+tilesize*4-tilesize/2)){
		ysel = 4-(((mousey+tilesize/2) - (screenHeight/2))/tilesize);
		if(ysel == 1){ ysel = -1;}
	}
	int sel = xsel + ysel*10;
	if((xsel < 0) || (xsel > 9) || (xraw < 0) || (ysel < 0) || (ysel > 59) || (mouseHidden)){
		sel = -1;
	}

	textMeshBox(guim,(screenWidth/2-5*tilesize)-tilesize/2,screenHeight/2-4*tilesize,11*tilesize,9*tilesize,23.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);

	guim->sx = (screenWidth/2-5*tilesize);
	guim->sy = screenHeight/2-4*tilesize+tilesize/2+tilesize/8;
	textMeshPrintf(guim,"Crafting");

	guim->sx = (screenWidth/2-5*tilesize);
	guim->sy = screenHeight/2-1*tilesize+tilesize/2+tilesize/8;
	textMeshPrintf(guim,"Inventory");

	for(int i = 0;i<40;i++){
		int x = (screenWidth/2-5*tilesize)+(i%10*tilesize);
		int y = screenHeight/2 + (4*tilesize) - (i/10*tilesize) - tilesize;
		if(i < 10){ y += tilesize/2; }
		int style = 0;
		if((i == sel) || (i == gamepadSelection)){
			style = 1;
		}
		textMeshItem(guim,x,y,tilesize, style, &player->inventory[i]);
	}

	for(int i=0;i<10;i++){
		if(i >= recipeGetCount()){break;}
		const int y = screenHeight/2 - tilesize*3;
		const int x = (screenWidth/2)+((i-5)*tilesize);
		int r = i;
		unsigned short b = recipeGetResultID(r);
		unsigned short a = recipeCanCraft(r,player);
		int style = 0;
		if((i == (sel-50)) || (i == (gamepadSelection-50))){
			style = 1;
		}else if(a <= 0){
			style = 2;
		}
		textMeshItemSlot(guim,x,y,tilesize,style,b,a);

		if(((mousex > x) && (mousex < x+tilesize) && (mousey > y) && (mousey < y+tilesize)) || (mouseHidden && (i == (gamepadSelection-50)))){
			const int yy = y + tilesize + tilesize/2;
			int ii,xx;

			for(ii=0;ii<4;ii++){
				xx = (screenWidth/2)+((ii*2-5)*tilesize);
				b = recipeGetIngredientID(r,ii);
				a = recipeGetIngredientAmount(r,ii);
				if((b == 0) || (a <= 0)){ break;}
				b = ingredientSubstituteGetSub(b,(ticks/96) % (ingredientSubstituteGetAmount(b)+1));

				if(ii > 0){
					textMeshBox(guim,xx-tilesize+tilesize/4+animX,yy+tilesize/4+animY,tilesize/2,tilesize/2,24.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
				}
				textMeshItemSlot(guim,xx,yy,tilesize,3,b,a);
			}
			b = recipeGetResultID(r);

			xx = (screenWidth/2)+((ii*2-5)*tilesize);
			textMeshBox(guim,xx-tilesize+tilesize/4+animX*2,yy+tilesize/4,tilesize/2,tilesize/2,25.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
			textMeshItemSlot(guim,xx,yy,tilesize,3,b,recipeGetResultAmount(r));
		}
	}

	if(mouseHidden){
		int gx,gy;
		inventoryGamepadSelToXY(gamepadSelection,&gx,&gy);
		textMeshItem(guim,gx+animX,gy+animY,tilesize,3,&inventoryCurrentPickup);
	}else{
		textMeshItem(guim,mousex+animX-tilesize/8,mousey+animY-tilesize/8,tilesize,3,&inventoryCurrentPickup);
	}
}

void inventoryClickOutside(int btn){
	if((btn == 1) && !itemIsEmpty(&inventoryCurrentPickup)){
		itemDropNewC(player, &inventoryCurrentPickup);
		itemDiscard(&inventoryCurrentPickup);
	}
}

void doInventoryClick(int btn, int sel){
	item *cItem;

	if((sel >= 50) && (sel < 60)){
		int r = sel-50;
		if(btn == 1){
			recipeDoCraft(r,player,1);
		}else if(btn == 3){
			recipeDoCraft(r,player,recipeCanCraft(r,player));
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
	if(gamepadSelection == -1){
		gamepadSelection = 0;
		return;
	}
	int xsel = gamepadSelection % 10;
	int ysel = gamepadSelection / 10;

	int xmax = 9;
	if(ysel == 5){xmax = MIN(9,recipeGetCount()-1);}
	xsel += x;
	if(xsel < 0){xsel = xmax;}
	if(xsel > xmax){xsel = 0;}
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
	gamepadSelection = xsel + ysel*10;
}

void updateInventoryGamepad(int btn){
	mouseHidden = true;
	if(gamepadSelection == -1){
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
