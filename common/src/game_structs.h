#pragma once
#include "gfx_structs.h"


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
	u8            texX[6];
	u8            texY[6];
	u32           color[2];
	mesh         *singleBlock;
	int           hp;
	blockCategory cat;
	char         *name;
} blockType;

typedef struct {
	u16 ID;
	i16 amount;
} item;

typedef uint32_t being;
inline  u8   beingType (being b){ return b>>24; }
inline u32   beingID   (being b){ return b&0xFFFFFF; }
inline being beingNew  (u8 type, u32 id){ return (id&0xFFFFFF) | ((u32)type << 24); }
#define BEING_MULL       0
#define BEING_CHARACTER  1
#define BEING_ANIMAL     2
#define BEING_HOOK       3
#define BEING_ENTITY     4
#define BEING_PROJECTILE 5
inline being beingCharacter (u32 id){ return beingNew(BEING_CHARACTER, id);}
inline being beingAnimal    (u32 id){ return beingNew(BEING_ANIMAL,    id);}
inline being beingHook      (u32 id){ return beingNew(BEING_HOOK,      id);}
inline being beingEntity    (u32 id){ return beingNew(BEING_ENTITY,    id);}
inline being beingProjectile(u32 id){ return beingNew(BEING_PROJECTILE,id);}

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
#define CHAR_FALLING      (1   )
#define CHAR_NOCLIP       (1<<1)
#define CHAR_COLLIDE      (1<<2)
#define CHAR_FALLINGSOUND (1<<3)
#define CHAR_SNEAK        (1<<4)
#define CHAR_GLIDE        (1<<5)
#define CHAR_SPAWNING     (1<<6)

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
	u8  clientOwner;

	u64 clientPriorization;

	chungus *curChungus;
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
} itemDrop;

typedef struct {
	vec pos,vel;
	being target,source;
	u16 style;
	i16 ttl;
} projectile;
