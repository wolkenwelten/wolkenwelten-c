#include "itemDrop.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../main.h"
#include "../network/server.h"
#include "../game/blockType.h"
#include "../../../common/src/misc.h"
#include "../../../common/src/packet.h"
#include "../../../common/src/messages.h"

typedef struct {
	entity    *ent;
	item       itm;
	float  aniStep;
} itemDrop;

itemDrop itemDrops[1<<16];
int      itemDropCount = 0;

void itemDropUpdatePlayer(int c){
	if(itemDropCount == 0){
		msgItemDropUpdatePlayer(c, 0.f, 0.f, 0.f, 0.f, 0, 0, 0, 0);
	}else{
		for(int i=0;i<itemDropCount;i++){
			msgItemDropUpdatePlayer(
				c,
				itemDrops[i].ent->x,
				itemDrops[i].ent->y,
				itemDrops[i].ent->z,
				itemDrops[i].aniStep,
				itemDrops[i].itm.ID,
				itemDrops[i].itm.amount,
				itemDropCount,
				i
			);
		}
	}
}

void itemDropNewP(float x, float y, float z,const item *itm){
	int d = itemDropCount++;

	itemDrops[d].itm = *itm;
	itemDrops[d].aniStep=0.f;

	itemDrops[d].ent = entityNew(x,y,z,0.f,0.f,0.f);
}

void itemDropNewC(const packet *p){
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
	itemDrops[d].aniStep    = 0.f;
}

void itemDropDel(int d){
	if(d < 0){return;}
	if(d >= itemDropCount){return;}
	entityFree(itemDrops[d].ent);
	itemDrops[d].ent = NULL;
	itemDrops[d] = itemDrops[--itemDropCount];
}

bool itemDropCheckPickup(int d){
	for(int i=0;i<clientCount;++i){
		float dx = clients[i].c->x - itemDrops[d].ent->x;
		float dy = clients[i].c->y - itemDrops[d].ent->y;
		float dz = clients[i].c->z - itemDrops[d].ent->z;
		if(sqrtf((dx*dx)+(dy*dy)+(dz*dz)) < 1.5f){
			msgPickupItem(i,itemDrops[d].itm.ID,itemDrops[d].itm.amount);
			return true;
		}
	}
	return false;
}

void itemDropUpdate(){
	for(int i=0;i<itemDropCount;i++){
		float aniStep = itemDrops[i].aniStep++;
		itemDrops[i].ent->yaw = aniStep / 4.f;
		itemDrops[i].ent->pitch = cosf(aniStep/ 96.f)*24;
		itemDrops[i].ent->yoff = (cosf(aniStep/192.f)/16.f)+0.1f;
		entityUpdate(itemDrops[i].ent);

		if(itemDropCheckPickup(i) || (itemDrops[i].ent->y < -256)){
			itemDropDel(i--);
			continue;
		}
	}
}
