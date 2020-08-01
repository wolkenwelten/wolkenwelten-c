#pragma once
#include "../gfx/mesh.h"
#include "../../../common/src/network/packet.h"

#include <stdbool.h>
#include <stdint.h>

struct grapplingHook;

typedef struct {
	uint16_t ID;
	 int16_t amount;
} item;

typedef struct {
	float  x, y, z;
	float vx,vy,vz;
	float gvx,gvy,gvz;
	float yaw,pitch,roll;
	float yoff,shake;
	float gyoff;
	float hitOff;

	bool falling;
	bool noClip;
	bool collide;
	bool fallingSound;
	bool sneak;
	bool hasHit;

	mesh *eMesh;
	struct grapplingHook *hook;

	short hp,maxhp;

	int blockMiningX,blockMiningY,blockMiningZ;

	unsigned int actionTimeout;
	unsigned int stepTimeout;

	unsigned int activeItem;
	item inventory[40];

	void *nextFree;
} character;

#include "../game/item.h"

extern character *player;

character *characterNew       ();
void  characterFree           (character *c);
bool  characterLOSBlock       (character *c, int *retX,int *retY,int *retZ,int returnBeforeBlock);
void  characterMineBlock      (character *c);
float characterMineProgress   (character *c);
void  characterStopMining     (character *c);
void  characterPlaceBlock     (character *c);
void  characterDie            (character *c);
bool  characterHP             (character *c, int addhp);
bool  characterDamage         (character *c, int hp);
int   characterGetItemAmount  (character *c, uint16_t itemID);
bool  characterDecItemAmount  (character *c, uint16_t itemID,int amount);
bool  characterPickupItem     (character *c, uint16_t itemID,int amount);
item *characterGetItemBarSlot (character *c, int i);
void  characterSetActiveItem  (character *c, int i);
void  characterDropItem       (character *c, int i);
void  characterSwapItemSlots  (character *c, int a,int b);
void  characterMove           (character *c, float mx,float my,float mz);
void  characterRotate         (character *c, float vYaw,float vPitch,float vRoll);
void  characterUpdate         (character *c);
void  characterFireHook       (character *c);
void  characterSetPos         (character *c, float x, float y, float z);
void  characterDraw           (character *c);
void  characterDrawAll        ();
uint32_t characterCollision   (character *c, float cx, float cy, float cz, float wd);
void  characterHitCheck       (character *c, int origin, float x, float y, float z, float yaw, float pitch, float roll, float pwr);
void  characterGotHitBroadcast(int c,float pwr);

void  characterMoveDelta      (character *c, packet *p);
void  characterDamagePacket   (character *c, packet *p);
void  characterSetPlayerPos   (const packet *p);
void  characterRemovePlayer   (int c, int len);
