#include "itemDrop.h"

#include "../main.h"
#include "../network/server.h"
#include "../game/blockType.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
	entity    *ent;
	item       itm;
} itemDrop;

itemDrop itemDrops[1<<14];
int      itemDropCount = 0;

#define ITEM_DROPS_PER_UPDATE 32
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

unsigned int itemDropUpdatePlayer(int c, unsigned int offset){
	const int max = MIN((int)(offset+ITEM_DROPS_PER_UPDATE),itemDropCount);
	for(int i=offset;i<max;i++){
		msgItemDropUpdate(\
			c,
			itemDrops[i].ent->x,
			itemDrops[i].ent->y,
			itemDrops[i].ent->z,
			itemDrops[i].ent->vx,
			itemDrops[i].ent->vy,
			itemDrops[i].ent->vz,
			i
		);
	}
	offset += ITEM_DROPS_PER_UPDATE;
	if((int)offset >= itemDropCount){offset=0;}
	return offset;
}

void itemDropNewP(float x, float y, float z,const item *itm){
	if(itm == NULL){return;}
	if((itemDropCount) >= (int)(sizeof(itemDrops) / sizeof(itemDrop) - 1)){return;}
	int d = itemDropCount++;

	itemDrops[d].itm     = *itm;
	itemDrops[d].ent     = entityNew(x,y,z,0.f,0.f,0.f);
	msgItemDropNew(-1, x, y, z, 0.f, 0.f, 0.f, itm->ID, itm->amount);
}

void itemDropNewC(const packet *p){
	if((itemDropCount) >= (int)(sizeof(itemDrops) / sizeof(itemDrop) - 1)){return;}
	int d = itemDropCount++;

	itemDrops[d].ent        = entityNew(0.f,0.f,0.f,0.f,0.f,0.f);
	itemDrops[d].ent->x     = p->val.f[0];
	itemDrops[d].ent->y     = p->val.f[1];
	itemDrops[d].ent->z     = p->val.f[2];
	itemDrops[d].ent->vx    = p->val.f[3];
	itemDrops[d].ent->vy    = p->val.f[4];
	itemDrops[d].ent->vz    = p->val.f[5];
	itemDrops[d].itm.ID     = p->val.i[6];
	itemDrops[d].itm.amount = p->val.i[7];

	msgItemDropNew(
		-1,
		itemDrops[d].ent->x,
		itemDrops[d].ent->y,
		itemDrops[d].ent->z,
		itemDrops[d].ent->vx,
		itemDrops[d].ent->vy,
		itemDrops[d].ent->vz,
		itemDrops[d].itm.ID,
		itemDrops[d].itm.amount
	);
}

void itemDropDel(int d){
	if(d < 0){return;}
	if(d >= itemDropCount){return;}
	entityFree(itemDrops[d].ent);
	itemDrops[d].ent = NULL;
	itemDrops[d] = itemDrops[--itemDropCount];

	msgItemDropDel(d);
}

bool itemDropCheckPickup(int d){
	for(int i=0;i<clientCount;++i){
		float dx = clients[i].c->x - itemDrops[d].ent->x;
		float dy = clients[i].c->y - itemDrops[d].ent->y;
		float dz = clients[i].c->z - itemDrops[d].ent->z;
		if(((dx*dx)+(dy*dy)+(dz*dz)) < (1.5f*1.5f)){
			msgPickupItem(i,itemDrops[d].itm.ID,itemDrops[d].itm.amount);
			return true;
		}
	}
	return false;
}

void itemDropUpdate(){
	for(int i=0;i<itemDropCount;i++){
		entityUpdate(itemDrops[i].ent);
		if(itemDropCheckPickup(i) || (itemDrops[i].ent->y < -256)){
			itemDropDel(i--);
			continue;
		}
	}
}

void itemDropIntro(int c){
	for(int d=0;d<itemDropCount;d++){
		msgItemDropNew(
			c,
			itemDrops[d].ent->x,
			itemDrops[d].ent->y,
			itemDrops[d].ent->z,
			itemDrops[d].ent->vx,
			itemDrops[d].ent->vy,
			itemDrops[d].ent->vz,
			itemDrops[d].itm.ID,
			itemDrops[d].itm.amount
		);
	}
}
