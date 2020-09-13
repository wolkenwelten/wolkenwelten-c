#include "itemDrop.h"
#include "../main.h"
#include "../game/blockType.h"
#include "../game/entity.h"
#include "../gfx/texture.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	entity    *ent;
	item       itm;
	uint32_t aniStep;
} itemDrop;

itemDrop itemDrops[1<<12];
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

	msgItemDropNew(-1,x,y,z,vx,vy,vz,itm->ID,itm->amount);
}

void itemDropNewD(float x, float y, float z, item *itm){
	const float vx = rngValf()*0.03f;
	const float vy = rngValf()*0.03f;
	const float vz = rngValf()*0.03f;
	msgItemDropNew(-1,x,y,z,vx,vy,vz,itm->ID,itm->amount);
}

void itemDropUpdate(){
	for(int i=0;i<itemDropCount;i++){
		float aniStep = ++itemDrops[i].aniStep;
		if(itemDrops[i].ent == NULL){continue;}
		itemDrops[i].ent->yaw   = aniStep / 4.f;
		itemDrops[i].ent->pitch = cosf(aniStep/ 96.f)*24;
		itemDrops[i].ent->yoff  = (cosf(aniStep/192.f)/16.f)+0.1f;
	}
}

void itemDropUpdateFromServer(packet *p){
	uint16_t d   = p->val.s[0];
	uint16_t len = p->val.s[1];

	if(len < itemDropCount){
		for(int i=len;i<itemDropCount;i++){
			if(itemDrops[i].ent != NULL){
				entityFree(itemDrops[i].ent);
				itemDrops[i].ent = NULL;
			}
		}
	}
	itemDropCount = len;

	if(itemDrops[d].ent == NULL) {
		itemDrops[d].ent = entityNew(0.f,0.f,0.f,0.f,0.f,0.f);
	}
	if(itemDrops[d].aniStep == 0){
		itemDrops[d].aniStep = rngValM(1024);
	}
	itemDrops[d].itm.ID     = p->val.s[2];
	itemDrops[d].itm.amount = p->val.s[3];

	itemDrops[d].ent->eMesh = getMeshDispatch(&itemDrops[d].itm);
	itemDrops[d].ent->x     = p->val.f[2];
	itemDrops[d].ent->y     = p->val.f[3];
	itemDrops[d].ent->z     = p->val.f[4];
	itemDrops[d].ent->vx    = p->val.f[5];
	itemDrops[d].ent->vy    = p->val.f[6];
	itemDrops[d].ent->vz    = p->val.f[7];
}
