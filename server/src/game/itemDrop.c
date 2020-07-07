#include "itemDrop.h"

#include <stdlib.h>
#include <math.h>

#include "../network/packet.h"
#include "../network/messages.h"
#include "../network/server.h"
#include "../game/blockType.h"
#include "../../../common/src/misc.h"
#include "../main.h"

typedef struct {
	entity    *ent;
	item       itm;
	float  aniStep;
} itemDrop;

itemDrop itemDrops[1<<16];
int      itemDropCount = 0;

void itemDropUpdatePlayer(int c){
	packetMedium p;
	for(int i=0;i<itemDropCount;i++){
		p.target = i;
		p.val.f[0] = itemDrops[i].ent->x;
		p.val.f[1] = itemDrops[i].ent->y;
		p.val.f[2] = itemDrops[i].ent->z;
		p.val.f[3] = itemDrops[i].aniStep;
		p.val.i[4] = itemDrops[i].itm.ID;
		p.val.i[5] = itemDrops[i].itm.amount;
		p.val.i[6] = itemDropCount;
		packetQueueM(&p,1,c);
	}
	if(itemDropCount == 0){
		p.val.i[6] = 0;
		packetQueueM(&p,1,c);
	}
}

void itemDropNewP(float x, float y, float z,const item *itm){
	int d = itemDropCount++;

	itemDrops[d].itm = *itm;
	itemDrops[d].aniStep=0.f;

	itemDrops[d].ent = entityNew(x,y,z,0.f,0.f,0.f);
}

void itemDropNewC(const packetMedium *p){
	int d = itemDropCount++;

	itemDrops[d].itm.ID     = p->val.i[6];
	itemDrops[d].itm.amount = p->target;
	itemDrops[d].aniStep=0.f;

	itemDrops[d].ent = entityNew(0.f,0.f,0.f,0.f,0.f,0.f);
	itemDrops[d].ent->x  = p->val.f[0];
	itemDrops[d].ent->y  = p->val.f[1];
	itemDrops[d].ent->z  = p->val.f[2];
	itemDrops[d].ent->vx = p->val.f[3];
	itemDrops[d].ent->vy = p->val.f[4];
	itemDrops[d].ent->vz = p->val.f[5];
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
			msgPickupItem(i,&itemDrops[d].itm);
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
