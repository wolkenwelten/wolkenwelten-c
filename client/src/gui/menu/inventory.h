#pragma once
#include "../../../../common/src/common.h"
#include "../widget.h"

extern widget *inventoryPanel;
extern item inventoryCurrentPickup;

void initInventory       ();
bool isInventoryOpen     ();
void drawInventory       (textMesh *guim);
void inventoryCheckCursorItem();

void openCrafting        ();
void closeCrafting       ();

void openInventory       ();
void closeInventory      ();
void toggleInventory     ();

void openInventoryPanel  ();
void closeInventoryPanel ();
