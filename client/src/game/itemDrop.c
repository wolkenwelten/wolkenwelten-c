#include "itemDrop.h"
#include "../main.h"
#include "../game/blockType.h"
#include "../gfx/texture.h"
#include "../mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdlib.h>

typedef struct {
	entity    *ent;
	item       itm;
	int    aniStep;
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

void itemDropDel(int d){
	entityFree(itemDrops[d].ent);
	itemDrops[d].ent = NULL;
	itemDrops[d] = itemDrops[--itemDropCount];
}

void itemDropUpdate(){
	for(int i=0;i<itemDropCount;i++){
		int aniStep = ++itemDrops[i].aniStep;
		if(itemDrops[i].ent == NULL){continue;}
		itemDrops[i].ent->yaw = aniStep / 4.f;
		itemDrops[i].ent->pitch = cosf(aniStep/ 96.f)*24;
		itemDrops[i].ent->yoff = (cosf(aniStep/192.f)/16.f)+0.1f;
	}
}

void itemDropNewFromServer(packet *p){
	int index = itemDropCount++;
	itemDrops[index].aniStep    = rngValM(1024);
	itemDrops[index].ent        = entityNew(0.f,0.f,0.f,0.f,0.f,0.f);
	itemDrops[index].ent->x     = p->val.f[0];
	itemDrops[index].ent->y     = p->val.f[1];
	itemDrops[index].ent->z     = p->val.f[2];
	itemDrops[index].ent->vx    = p->val.f[3];
	itemDrops[index].ent->vy    = p->val.f[4];
	itemDrops[index].ent->vz    = p->val.f[5];
	itemDrops[index].itm.ID     = p->val.i[6];
	itemDrops[index].itm.amount = p->val.i[7];
	itemDrops[index].ent->eMesh = getMeshDispatch(&itemDrops[index].itm);
}

void itemDropUpdateFromServer(packet *p){
	int index = p->val.i[6];
	if(itemDrops[index].ent == NULL){return;}
	itemDrops[index].ent->x  = p->val.f[0];
	itemDrops[index].ent->y  = p->val.f[1];
	itemDrops[index].ent->z  = p->val.f[2];
	itemDrops[index].ent->vx = p->val.f[3];
	itemDrops[index].ent->vy = p->val.f[4];
	itemDrops[index].ent->vz = p->val.f[5];

}
