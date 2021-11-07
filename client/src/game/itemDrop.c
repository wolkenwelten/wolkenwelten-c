/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "itemDrop.h"

#include "../game/character.h"
#include "../game/entity.h"
#include "../sdl/sfx.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include <math.h>
#include <stdio.h>

void itemDropNewC(const character *chr,const item *itm){
	const vec vel = vecMulS(vecDegToVec(chr->rot),0.03f);
	const vec pos = vecAdd(vecAdd(chr->pos,vecNew(0,0.5,0)),vecMulS(vel,6.f));
	msgItemDropNew(-1,vecAdd(pos,vecMulS(vel,100)),vel,itm);
}

void itemDropNewP(const vec pos,const item *itm, i16 IDPlayer){
	(void)IDPlayer;
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
		if(itemDropList[i].player != -1){
			if(dd > 3.f * 3.f){
				itemDropList[i].player = -1;
			}
		}else if(dd < 3.f * 3.f){
			const float vel    = vecMag(itemDropList[i].ent->vel);
			const float weight = itemGetWeight(&itemDropList[i].itm);
			const float vw     = vel * weight;
			if(vw > 20.f){
				msgItemDropBounce(-1, i);
				itemDropList[i].player = playerID;
				sfxPlay(sfxImpact,1.0f);
				if(vw > 4.f){
					characterDamage(player,(int)vw);
				}
			}else{
				msgItemDropPickup(-1, i);
				itemDropList[i].player = playerID;
				sfxPlay(sfxPock,0.5f);
			}
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
			itemDropList[i].player = -1;
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
		itemDropList[d].player = -1;
		return;
	}
	const vec idpos = vecNewP(&p->v.f[2]);
	const vec idvel = vecNewP(&p->v.f[5]);
	if(itemDropList[d].ent == NULL){
		itemDropList[d].ent     = entityNew(idpos,vecZero(),itemGetWeight(&itemDropList[d].itm));
		itemDropList[d].aniStep = rngValM(1024);
		itemDropList[d].player  = p->v.i16[16];
	}

	itemDropList[d].ent->eMesh = itemGetMesh(&itemDropList[d].itm);
	itemDropList[d].ent->pos   = idpos;
	itemDropList[d].ent->vel   = idvel;
}
