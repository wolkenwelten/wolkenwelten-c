#include "itemDrop.h"
#include "../main.h"
#include "../game/blockType.h"
#include "../gfx/texture.h"
#include "../misc/misc.h"
#include "../network/messages.h"

#include <math.h>
#include <stdlib.h>

typedef struct {
	entity    *ent;
	item       itm;
	float  aniStep;
} itemDrop;

itemDrop itemDrops[1<<16];
int      itemDropCount = 0;

void itemDropNewC(character *chr, item *itm){
	float x,y,z,vx,vy,vz;
	x = chr->x;
	y = chr->y + 0.4f;
	z = chr->z;
	vx = (cos((chr->yaw-90.f)*PI/180) * cos(-chr->pitch*PI/180))*0.03f;
	vy = sin(-chr->pitch*PI/180)*0.03f;
	vz = (sin((chr->yaw-90.f)*PI/180) * cos(-chr->pitch*PI/180))*0.03f;
	x += vx * 60.f;
	y += vy * 60.f;
	z += vz * 60.f;

	msgItemDropNew(x,y,z,vx,vy,vz,itm->ID,itm->amount);
}

void itemDropDel(int d){
	entityFree(itemDrops[d].ent);
	itemDrops[d].ent = NULL;
	itemDrops[d] = itemDrops[--itemDropCount];
}

void itemDropUpdate(){
	for(int i=0;i<itemDropCount;i++){
		float aniStep = ++itemDrops[i].aniStep;
		itemDrops[i].ent->yaw = aniStep / 4.f;
		itemDrops[i].ent->pitch = cosf(aniStep/ 96.f)*24;
		itemDrops[i].ent->yoff = (cosf(aniStep/192.f)/16.f)+0.1f;
	}
}

void itemDropUpdateFromServer(packetMedium *p){
	for(int i=p->val.i[6];i<itemDropCount;i++){
		if(itemDrops[i].ent != NULL){
			entityFree(itemDrops[i].ent);
		}
		itemDrops[i].ent = NULL;
	}
	itemDropCount = p->val.i[6];
	if(p->target >= itemDropCount){return;}
	if(itemDrops[p->target].ent == NULL){
		itemDrops[p->target].ent = entityNew(0.f,0.f,0.f,0.f,0.f,0.f);
	}
	itemDrops[p->target].ent->x     = p->val.f[0];
	itemDrops[p->target].ent->y     = p->val.f[1];
	itemDrops[p->target].ent->z     = p->val.f[2];
	itemDrops[p->target].aniStep    = p->val.f[3];
	itemDrops[p->target].itm.ID     = p->val.i[4];
	itemDrops[p->target].itm.amount = p->val.i[5];
	itemDrops[p->target].ent->eMesh = itemGetMesh(&itemDrops[p->target].itm);
}
