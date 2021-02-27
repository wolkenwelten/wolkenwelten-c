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
	float inaccuracy;
} itemType;

typedef uint32_t being;
inline  u8   beingType (being b){ return b>>24; }
inline u32   beingID   (being b){ return b&0xFFFFFF; }
inline being beingNew  (u8 type, u32 id){ return (id&0xFFFFFF) | ((u32)type << 24); }
#define BEING_NULL       0
#define BEING_CHARACTER  1
#define BEING_ANIMAL     2
#define BEING_HOOK       3
#define BEING_GRENADE    4
#define BEING_PROJECTILE 5
#define BEING_ITEMDROP   6
#define BEING_FIRE       7

#define BEING_THROWABLE  9

inline being beingCharacter (u32 id){ return beingNew(BEING_CHARACTER, id);}
inline being beingAnimal    (u32 id){ return beingNew(BEING_ANIMAL,    id);}
inline being beingHook      (u32 id){ return beingNew(BEING_HOOK,      id);}
inline being beingGrenade   (u32 id){ return beingNew(BEING_GRENADE,   id);}
inline being beingProjectile(u32 id){ return beingNew(BEING_PROJECTILE,id);}
inline being beingItemDrop  (u32 id){ return beingNew(BEING_ITEMDROP,  id);}
inline being beingFire      (u32 id){ return beingNew(BEING_FIRE,      id);}
inline being beingThrowable (u32 id){ return beingNew(BEING_THROWABLE, id);}

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

typedef struct {
	 vec  pos,vel,gvel,rot,screenPos;
	float yoff,shake,inaccuracy;
	float gyoff;

	 int  animationIndex;
	 int  animationTicksMax;
	 int  animationTicksLeft;
	uint  breathing;
        uint  temp;

	 u32  flags;

	float gliderFade;
	float aimFade;
	float zoomFactor;

	mesh *eMesh;
	hook *hook;

	 i16 hp,maxhp;

	 vec hookPos;
	 int blockMiningX,blockMiningY,blockMiningZ;

	 int actionTimeout;
	uint stepTimeout;

	uint activeItem;
	item inventory[40];
	item equipment[3];

	void *nextFree;
} character;
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

#define CHAR_EQ_GLIDER     0
#define CHAR_EQ_HOOK       1
#define CHAR_EQ_PACK       2

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
