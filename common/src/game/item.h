#pragma once
#include "../common.h"

item  itemNew        (u16 ID, i16 amount);
item  itemEmpty      ();
void  itemDiscard    (      item *i);
bool  itemIsEmpty    (const item *i);

int   itemCanStack   (const item *i, u16 ID);
int   itemIncStack   (      item *i, i16 amount);
int   itemDecStack   (      item *i, i16 amount);

int   itemGetAmmo    (const item *i);
int   itemIncAmmo    (      item *i, i16 amount);
int   itemDecAmmo    (      item *i, i16 amount);
