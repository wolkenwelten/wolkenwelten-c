#pragma once
#include "../../../common/src/common.h"

void showInventory  ();
void hideInventory  ();
bool isInventoryOpen();

void initInventory            ();
void drawInventory            (textMesh *guim);
void updateInventoryClick     (int x,int y, int btn);
void changeInventorySelection (int x,int y);
void updateInventoryGamepad   (int btn);
