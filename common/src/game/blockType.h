#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/misc/side.h"

extern blockId blockTypeMax;
extern u8 blockLight[256];

void          blockTypeInit();

// ToDo: just return the blockType struct directly!
const char   *blockTypeGetName            (blockId b);
int           blockTypeGetHealth          (blockId b);
int           blockTypeGetFireHealth      (blockId b);
int           blockTypeGetFireDamage      (blockId b);
blockCategory blockTypeGetCat             (blockId b);
float         blockTypeGetWeight          (blockId b);
bool          blockTypeValid              (blockId b);
u16           blockTypeGetTexX            (blockId b, side side);
u16           blockTypeGetTexY            (blockId b, side side);
u32           blockTypeGetParticleColor   (blockId b);
mesh         *blockTypeGetMesh            (blockId b);
u16           blockTypeGetIngressMask     (blockId b);
u16           blockTypeGetEgressMask      (blockId b);
u8            blockTypeGetLightEmission   (blockId b);


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
#define I_Snow_Dirt     23
#define I_Snow_Grass    24
#define I_Snow_Dirt     23
#define I_Snowy_Spruce_Leaf 25
#define I_Snowy_Oak_Leaf    26
#define I_Snowy_Flower      27
#define I_Snowy_Date        28
#define I_Snowy_Acacia_Leaf 29
#define I_Snowy_Roots       30
#define I_Snowy_Sakura_Leaf 31
