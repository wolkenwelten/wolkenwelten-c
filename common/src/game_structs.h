#pragma once
#include "gfx_structs.h"

typedef enum blockCategory {
	NONE,
	DIRT,
	STONE,
	WOOD,
	LEAVES
} blockCategory;

typedef struct {
	unsigned char texX[6];
	unsigned char texY[6];
	unsigned int color[2];
	mesh *singleBlock;
	int hp;
	blockCategory cat;
	char *name;
} blockType;

typedef struct {
	uint16_t ID;
	 int16_t amount;
} item;

struct grapplingHook;
typedef struct grapplingHook grapplingHook;

struct bigchungus;
typedef struct bigchungus bigchungus;
struct chungus;
typedef struct chungus chungus;
struct chunk;
typedef struct chunk chunk;

typedef struct {
	float  x, y, z;
	float vx,vy,vz;
	float yaw,pitch,roll;
	float yoff;
	uint32_t flags;

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
	float  x, y, z;
	float vx,vy,vz;
	float gvx,gvy,gvz;
	float yaw,pitch,roll;
	float yoff,shake,inaccuracy;
	float gyoff;

	int animationIndex;
	int animationTicksMax;
	int animationTicksLeft;
	unsigned int breathing;

	uint32_t flags;
	float gliderFade;

	mesh *eMesh;
	grapplingHook *hook;

	short hp,maxhp;

	float hookx,hooky,hookz;
	int blockMiningX,blockMiningY,blockMiningZ;

	int actionTimeout;
	unsigned int stepTimeout;

	unsigned int activeItem;
	item inventory[40];

	void *nextFree;
} character;
#define CHAR_FALLING      (1   )
#define CHAR_NOCLIP       (1<<1)
#define CHAR_COLLIDE      (1<<2)
#define CHAR_FALLINGSOUND (1<<3)
#define CHAR_SNEAK        (1<<4)
#define CHAR_GLIDE        (1<<5)

typedef struct {
	float  x, y, z;
	float vx,vy,vz;
	float yaw,pitch,roll;

	float gyaw,gpitch;
	float gvx,gvy,gvz;

	int8_t age;
	int8_t health;
	int8_t hunger;
	int8_t thirst;
	int8_t sleepy;

	unsigned int breathing;

	uint8_t flags;
	uint8_t type;
	uint8_t state;

	chungus *curChungus;
} animal;
#define ANIMAL_FALLING    (1   )
#define ANIMAL_BELLYSLEEP (1<<1)
#define ANIMAL_COLLIDE    (1<<3)

#define ANIMAL_S_LOITER    0
#define ANIMAL_S_FLEE      1
#define ANIMAL_S_HEAT      2
#define ANIMAL_S_SLEEP     3
#define ANIMAL_S_PLAYING   4

struct grapplingHook {
	entity       *ent;
	mesh        *rope;
	character *parent;
	float  goalLength;
	bool       hooked;
	bool    returning;
};
