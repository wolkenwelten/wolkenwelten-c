#pragma once
#include "gfx_structs.h"
#include "misc/side.h"

#define CHUNK_COORDS (16)
#define CHUNK_SIZE   (16)

extern u64 gameTicks;

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

typedef enum deathCause {
	deathCauseCommand = 0,
	deathCauseBeamblast,
	deathCauseMelee,
	deathCauseProjectile,
	deathCausePhysics,
	deathCauseAbyss,
	deathCauseFire,
	deathCauseGrenade,
	deathCauseLightning
} deathCause;

typedef struct {
	u8            tex[sideMAX];
	u8            texX[sideMAX];
	u8            texY[sideMAX];
	u32           color[2];
	mesh         *singleBlock;
	int           hp,firehp;
	blockCategory cat;
	char         *name;
	float         weight;
	u16           ingressMask;
	u16           egressMask;
	u8            lightEmission;
} blockType;
typedef u8 blockId;

typedef enum chunkOverlayType {
	chunkOverlayBlock = 0,
	chunkOverlayFluid,
	chunkOverlayFire
} chunkOverlayType;

typedef struct {
	union {
		u8 data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
		void *nextFree;
	};
} chunkOverlay;

typedef uint32_t being;
#define BEING_NULL       0
#define BEING_CHARACTER  1
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
	u32 generation;

	mesh    *mesh;
	chungus *curChungus;
	lVal    *handler;
	void    *nextFree;
} entity;
#define ENTITY_FALLING     (1   )
#define ENTITY_NOCLIP      (1<<1)
#define ENTITY_UPDATED     (1<<2)
#define ENTITY_COLLIDE     (1<<3)
#define ENTITY_NOREPULSION (1<<4)
#define ENTITY_SLOW_UPDATE (1<<5)
#define ENTITY_HIDDEN      (1<<6)

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

typedef enum animType {
	animationHit = 0,
	animationFire,
	animationReload,
	animationEmpty,
	animationEat,
	animationSwitch
} animType;

#define CHAR_EQ_MAX    3
#define CHAR_INV_MAX  60

#define CHAR_EQ_GLIDER     0
#define CHAR_EQ_HOOK       1
#define CHAR_EQ_PACK       2

typedef struct {
	 vec  pos,vel,gvel,controls,rot;
	float yoff,shake,inaccuracy;
	float gyoff;

    animType  animationIndex;
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
          u8 effectValue;

	void *nextFree;
} character;

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
	vec pos,vel;
	being target,source;
	u16 style;
	i16 ttl;
} projectile;
