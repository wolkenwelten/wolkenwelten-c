#pragma once
#include "../../../common/src/common.h"
#include "../gui/widget.h"

extern widget *inventoryPanel;

void showCrafting       ();
void showInventory      ();
void hideInventory      ();
bool isInventoryOpen    ();
void initInventory      ();
void drawInventory      (textMesh *guim);
void showInventoryPanel ();
void hideInventoryPanel ();
