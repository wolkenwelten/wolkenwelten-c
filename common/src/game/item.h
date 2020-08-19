#pragma once
#include "../common.h"

item  itemNew        (uint16_t ID, int16_t amount);
item  itemEmpty      ();
void  itemDiscard    (item *i);
bool  itemIsEmpty    (item *i);

int   itemCanStack   (item *i, uint16_t ID);
int   itemIncStack   (item *i, int16_t amount);
int   itemDecStack   (item *i, int16_t amount);

int   itemGetAmmo    (item *i);
int   itemIncAmmo    (item *i, int16_t amount);
int   itemDecAmmo    (item *i, int16_t amount);
