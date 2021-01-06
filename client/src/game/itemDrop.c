#include "itemDrop.h"

#include "../game/character.h"
#include "../game/entity.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>

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

void itemDropUpdateAll(){
	PROFILE_START();

	for(uint i=0;i<itemDropCount;i++){
		float aniStep = ++itemDropList[i].aniStep;
		if(itemDropList[i].ent == NULL){continue;}
		itemDropList[i].ent->rot  = vecNew(aniStep/4.f,cosf(aniStep/96.f)*24.f,0);
		itemDropList[i].ent->yoff = (cosf(aniStep/192.f)/16.f)+0.1f;
		const vec dist = vecSub(player->pos,itemDropList[i].ent->pos);
		const float dd = vecDot(dist,dist);
		if(itemDropList[i].player == playerID){
			if(dd > 2.f * 2.f){itemDropList[i].player = -1;}
		}else if(dd < 1.5f*1.5f){
			msgItemDropPickup(-1, i);
			itemDropList[i].player = playerID;
		}
	}

	PROFILE_STOP();
}

void itemDropUpdateFromServer(const packet *p){
	const u16 d   = p->v.u16[0];
	const u16 len = p->v.u16[1];

	if(len < itemDropCount){
		for(uint i=MAX(0,len-1);i<itemDropCount;i++){
			if(itemDropList[i].ent == NULL){continue;}
			entityFree(itemDropList[i].ent);
			itemDropList[i].ent = NULL;
		}
	}
	itemDropCount = len;
	if(d >= len){return;}

	itemDropList[d].itm.ID     = p->v.u16[2];
	itemDropList[d].itm.amount = p->v.i16[3];
	if(itemIsEmpty(&itemDropList[d].itm)){
		if(itemDropList[d].ent == NULL) { return; }
		entityFree(itemDropList[d].ent);
		itemDropList[d].ent = NULL;
		return;
	}
	if(itemDropList[d].ent == NULL) {
		itemDropList[d].ent     = entityNew(vecZero(),vecZero());
		itemDropList[d].aniStep = rngValM(1024);
	}

	itemDropList[d].ent->eMesh = getMeshDispatch(&itemDropList[d].itm);
	itemDropList[d].ent->pos   = vecNewP(&p->v.f[2]);
	itemDropList[d].ent->vel   = vecNewP(&p->v.f[5]);
}
