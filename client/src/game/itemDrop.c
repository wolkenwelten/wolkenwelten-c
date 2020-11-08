#include "itemDrop.h"

#include "../game/character.h"
#include "../game/entity.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>

itemDrop itemDrops[1<<14];
int      itemDropCount = 0;

void itemDropNewC(const character *chr,const item *itm){
	const vec pos = vecAdd(chr->pos,vecNew(0,0.4,0));
	const vec vel = vecMulS(vecDegToVec(chr->rot),0.03f);
	msgItemDropNew(-1,vecAdd(pos,vecMulS(vel,60)),vel,itm);
}

void itemDropNewP(const vec pos,const item *itm){
	if(itemIsEmpty(itm)){return;}
	const vec vel = vecMulS(vecRngAbs(),0.03);
	msgItemDropNew(-1,pos,vel,itm);
}

void itemDropUpdate(){
	for(int i=0;i<itemDropCount;i++){
		float aniStep = ++itemDrops[i].aniStep;
		if(itemDrops[i].ent == NULL){continue;}
		itemDrops[i].ent->rot  = vecNew(aniStep/4.f,cosf(aniStep/96.f)*24.f,0);
		itemDrops[i].ent->yoff = (cosf(aniStep/192.f)/16.f)+0.1f;
		const vec dist = vecSub(player->pos,itemDrops[i].ent->pos);
		const float dd = vecDot(dist,dist);
		if(itemDrops[i].player == playerID){
			if(dd > 2.f * 2.f){itemDrops[i].player = -1;}
		}else if(dd < 1.5f*1.5f){
			msgItemDropPickup(-1, i);
			itemDrops[i].player = playerID;
		}
	}
}

void itemDropUpdateFromServer(const packet *p){
	const u16 d   = p->v.u16[0];
	const u16 len = p->v.u16[1];

	if(len < itemDropCount){
		for(int i=MAX(0,len-1);i<itemDropCount;i++){
			if(itemDrops[i].ent == NULL){continue;}
			entityFree(itemDrops[i].ent);
			itemDrops[i].ent = NULL;
		}
	}
	itemDropCount = len;
	if(d >= len){return;}

	itemDrops[d].itm.ID     = p->v.u16[2];
	itemDrops[d].itm.amount = p->v.i16[3];
	if(itemIsEmpty(&itemDrops[d].itm)){
		if(itemDrops[d].ent == NULL) { return; }
		entityFree(itemDrops[d].ent);
		itemDrops[d].ent = NULL;
		return;
	}
	if(itemDrops[d].ent == NULL) {
		itemDrops[d].ent     = entityNew(vecZero(),vecZero());
		itemDrops[d].aniStep = rngValM(1024);
	}

	itemDrops[d].ent->eMesh = getMeshDispatch(&itemDrops[d].itm);
	itemDrops[d].ent->pos   = vecNewP(&p->v.f[2]);
	itemDrops[d].ent->vel   = vecNewP(&p->v.f[5]);
}
