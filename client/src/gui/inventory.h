#pragma once
#include "../gfx/textMesh.h"

#include <stdbool.h>

void showInventory();
void hideInventory();
bool isInventoryOpen();

void drawInventory(textMesh *guim);
void updateInventoryClick(int x,int y, int btn);
void changeInventorySelection(int x,int y);
void updateInventoryGamepad(int btn);
