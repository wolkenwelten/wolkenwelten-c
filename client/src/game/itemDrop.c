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

itemDrop itemDrops[1<<12];
int      itemDropCount = 0;

void itemDropNewC(const character *chr,const item *itm){
	const vec pos = vecAdd(chr->pos,vecNew(0,0.4,0));
	const vec vel = vecMulS(vecDegToVec(chr->rot),0.03f);
	msgItemDropNew(-1,vecAdd(pos,vecMulS(vel,60)),vel,itm);
}

void itemDropNewD(const vec pos,const item *itm){
	const vec vel = vecMulS(vecRngAbs(),0.03);
	msgItemDropNew(-1,pos,vel,itm);
}

void itemDropUpdate(){
	for(int i=0;i<itemDropCount;i++){
		float aniStep = ++itemDrops[i].aniStep;
		if(itemDrops[i].ent == NULL){continue;}
		itemDrops[i].ent->rot  = vecNew(aniStep/4.f,cosf(aniStep/96.f)*24.f,0);
		itemDrops[i].ent->yoff = (cosf(aniStep/192.f)/16.f)+0.1f;
	}
}

void itemDropUpdateFromServer(const packet *p){
	u16 d   = p->val.s[0];
	u16 len = p->val.s[1];

	if(len < itemDropCount){
		for(int i=len-1;i<itemDropCount;i++){
			if(itemDrops[i].ent != NULL){
				entityFree(itemDrops[i].ent);
				itemDrops[i].ent = NULL;
			}
		}
	}
	itemDropCount = len;
	if(d >= len){return;}

	if(itemDrops[d].ent == NULL) {
		itemDrops[d].ent     = entityNew(vecZero(),vecZero());
		itemDrops[d].aniStep = rngValM(1024);
	}
	itemDrops[d].itm.ID     = p->val.s[2];
	itemDrops[d].itm.amount = p->val.s[3];

	itemDrops[d].ent->eMesh = getMeshDispatch(&itemDrops[d].itm);
	itemDrops[d].ent->pos   = vecNewP(&p->val.f[2]);
	itemDrops[d].ent->vel   = vecNewP(&p->val.f[5]);
}
