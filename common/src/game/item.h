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

#define I_Dirt           1
#define I_Grass          2
#define I_Stone          3
#define I_Coal           4
#define I_Spruce         5
#define I_Spruce_Leaf    6
#define I_Roots          7
#define I_Dry_Grass      8
#define I_Obsidian       9
#define I_Oak           10
#define I_Oak_Leaf      11
#define I_Marble_Block  12
#define I_Hematite_Ore  13
#define I_Marble_Pillar 14
#define I_Marble_Blocks 15
#define I_Acacia_Leaf   16
#define I_Board         17
#define I_Crystal       18
#define I_Sakura_Leaf   19
#define I_Birch         20
#define I_Flower        21
#define I_Date          22

#define I_Grenade       256
#define I_Bomb          257
#define I_Pear          258
#define I_Stone_Axe     259
#define I_Stone_Pick    260
#define I_Blaster       261
#define I_MasterB       262
#define I_AssaultB      263
#define I_ShotgunB      264
#define I_Crystalbullet 265
#define I_Iron_Bar      266
#define I_Iron_Axe      267
#define I_Iron_Pick     268
#define I_Crystal_Bar   269
#define I_Crystal_Axe   270
#define I_Crystal_Pick  271
#define I_Cherry        272
#define I_ClusterBomb   273
#define I_Glider        274
#define I_Hook          275
#define I_Jetpack       276
#define I_Poop          277
#define I_Meat          278
#define I_Cookedmeat    279
#define I_Fur           280
#define I_Burntmeat     281
#define I_FlintAndSteel 282
#define I_Flamethrower  283
#define I_Flamebullet   284
#define I_Iron_Dust     285
#define I_Crystal_Dust  286
#define I_Flamestick    287
#define I_Waterthrower  288
#define I_Stonespear    289
