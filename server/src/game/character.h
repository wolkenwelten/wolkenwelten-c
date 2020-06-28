#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint16_t ID;
	 int16_t amount;
} item;

typedef struct {
	float   x,  y,  z;
	float  vx, vy, vz;
	float gvx,gvy,gvz;
	float yaw,pitch,roll;
	float yoff,gyoff;
	float hitOff;

	bool falling;
	bool noClip;
	bool collide;
	bool fallingSound;
	bool sneak;
	bool hasHit;

	bool hook;
	float hookx,hooky,hookz;

	short hp,maxhp;

	int blockMiningX,blockMiningY,blockMiningZ;

	unsigned int actionTimeout;
	unsigned int stepTimeout;

	unsigned int activeItem;
	item inventory[40];

	void *nextFree;
} character;

extern character characterList[128];
extern int characterCount;

character *characterNew       ();
void  characterFree           (      character *c);
void  characterInit           (      character *c);
bool  characterLOSBlock       (const character *c, int *retX, int *retY, int *retZ, int returnBeforeBlock);
void  characterDie            (      character *c);
bool  characterHP             (      character *c, int addhp);
int   characterGetItemAmount  (const character *c, uint16_t itemID);
bool  characterDecItemAmount  (      character *c, uint16_t itemID, int amount);
bool  characterPickupItem     (      character *c, uint16_t itemID, int amount);
item *characterGetItemBarSlot (const character *c, int i);
void  characterSetActiveItem  (      character *c, int i);
void  characterDropItem       (      character *c, int i);
uint32_t characterCollision   (const character *c, float cx, float cy, float cz,float wd);
