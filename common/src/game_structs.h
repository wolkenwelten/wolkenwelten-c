#pragma once
#include "gfx_structs.h"
#include "misc/side.h"


typedef struct sfx sfx;
typedef struct bgm bgm;
typedef struct hook hook;
typedef struct bigchungus bigchungus;
typedef struct chungus chungus;
typedef struct chunk chunk;

typedef enum blockCategory {
	NONE,
	DIRT,
	STONE,
	WOOD,
	LEAVES
} blockCategory;

typedef struct {
	u8            tex[sideMAX];
	u8            texX[sideMAX];
	u8            texY[sideMAX];
	u32           color[2];
	mesh         *singleBlock;
	int           hp,firehp;
	i16           waterIngress;
	i16           waterEgress;
	i16           waterCapacity;
	blockCategory cat;
	char         *name;
	float         weight;
} blockType;

typedef struct {
	u16 ID;
	i16 amount;
} item;

typedef struct {
	char name[32];
	mesh *iMesh;
	i16 damage[5];
	u16 ammunition,stackSize,magazineSize;
	i16 fireDamage;
	u16 fireHealth;
	u32 itemDropChance;
	float inaccuracy;
	float weight;

	i16 spriteIndex[4];
	u32 spriteColor[4];
} itemType;

typedef uint32_t being;
#define BEING_NULL       0
#define BEING_CHARACTER  1
#define BEING_ANIMAL     2
#define BEING_HOOK       3
#define BEING_GRENADE    4
#define BEING_PROJECTILE 5
#define BEING_ITEMDROP   6
#define BEING_FIRE       7

#define BEING_THROWABLE  9

typedef struct beingListEntry beingListEntry;
struct beingListEntry {
	beingListEntry *next;
	being v[14];
};

typedef struct beingList beingList;
struct beingList {
	beingListEntry *first;
	uint count;
	beingList *parent;
};

typedef struct {
	vec pos,vel,rot;
	float yoff;
	float weight;
	u32 flags;

	mesh    *eMesh;
	chungus *curChungus;
	void    *nextFree;
} entity;
#define ENTITY_FALLING     (1   )
#define ENTITY_NOCLIP      (1<<1)
#define ENTITY_UPDATED     (1<<2)
#define ENTITY_COLLIDE     (1<<3)
#define ENTITY_NOREPULSION (1<<4)

typedef struct {
	being a,b;
	float length;
	u16   flags;
} rope;
#define ROPE_TEX       0x0FF
#define ROPE_TEX_ROPE  0x000
#define ROPE_TEX_CHAIN 0x001
#define ROPE_DIRTY     0x100


#define CHAR_FALLING      (1    )
#define CHAR_NOCLIP       (1<< 1)
#define CHAR_COLLIDE      (1<< 2)
#define CHAR_FALLINGSOUND (1<< 3)
#define CHAR_SNEAK        (1<< 4)
#define CHAR_GLIDE        (1<< 5)
#define CHAR_SPAWNING     (1<< 6)
#define CHAR_JUMPING      (1<< 7)
#define CHAR_BOOSTING     (1<< 8)
#define CHAR_AIMING       (1<< 9)
#define CHAR_THROW_AIM    (1<<10)
#define CHAR_CONS_MODE    (1<<11)
#define CHAR_JUMP_NEXT    (1<<12)

#define CHAR_EQ_MAX    3
#define CHAR_INV_MAX  40

#define CHAR_EQ_GLIDER     0
#define CHAR_EQ_HOOK       1
#define CHAR_EQ_PACK       2

typedef struct {
	 vec  pos,vel,gvel,controls,rot,screenPos;
	float yoff,shake,inaccuracy;
	float gyoff;

	 int  animationIndex;
	 int  animationTicksMax;
	 int  animationTicksLeft;
	uint  breathing;
        uint  temp;

	 u32  flags;

	float gliderFade;
	float goalZoomFactor;
	float zoomFactor;

	mesh *eMesh;
	hook *hook;

	 i16 hp,maxhp;

	 vec hookPos;
	 int blockMiningX,blockMiningY,blockMiningZ;

	 int actionTimeout;
	uint stepTimeout;
	 int cloudyness;

	uint activeItem;
	item inventory[CHAR_INV_MAX];
	item equipment[CHAR_EQ_MAX];

	void *nextFree;
} character;

typedef struct {
	vec pos,vel,rot;
	vec gvel,grot;
	vec screenPos;

	i8 age;
	i8 health;
	i8 hunger;
	i8 pregnancy;
	i8 sleepy;
	u8 flags;
	u8 type;
	u8 state;

	u16 nextFree;
	u16 breathing;
	u16 temp;
	being target;

	u64 clientPriorization;
	u32 stateTicks;

	beingList *bl;
} animal;
#define ANIMAL_FALLING    (1   )
#define ANIMAL_BELLYSLEEP (1<<1)
#define ANIMAL_AGGRESIVE  (1<<2)
#define ANIMAL_COLLIDE    (1<<3)
#define ANIMAL_MALE       (1<<4)
#define ANIMAL_NO_NEEDS   (1<<5)

#define ANIMAL_S_LOITER      0
#define ANIMAL_S_FLEE        1
#define ANIMAL_S_HEAT        2
#define ANIMAL_S_SLEEP       3
#define ANIMAL_S_PLAYING     4
#define ANIMAL_S_FOOD_SEARCH 5
#define ANIMAL_S_EAT         6
#define ANIMAL_S_FIGHT       7
#define ANIMAL_S_HUNT        8

struct hook {
	entity       *ent;
	character *parent;
	rope        *rope;
	being    attached;
	float  goalLength;
	bool       hooked;
	bool    returning;
};

typedef struct {
	entity    *ent;
	item       itm;
	union {
		u16 aniStep;
		i16 nextFree;
	};
	i16     player;

	u16 lastFire;
	i16 fireDmg;
} itemDrop;


typedef struct {
	entity     *ent;
	item        itm;
	i16    nextFree;
	u16     counter;
	u8        flags;
	i8       damage;
	being   thrower;
} throwable;

#define THROWABLE_PITCH_SPIN   (1   )
#define THROWABLE_TIP_HEAVY    (1<<1)
#define THROWABLE_PIERCE       (1<<2)
#define THROWABLE_COLLECTABLE  (1<<3)
#define THROWABLE_BURNING      (1<<4)

typedef struct {
	vec pos,vel;
	being target,source;
	u16 style;
	i16 ttl;
} projectile;

typedef struct {
	u16 x,y,z;
	i16 strength;
	i16 blockDmg;
	i16 oxygen;
	beingList *bl;
} fire;

typedef struct {
	entity *ent;
	int ticksLeft,cluster;
	float pwr,clusterPwr;
} grenade;
