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

#include "../game/fire.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/game/entity.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/lisp.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"
#include "../../../common/src/network/messages.h"

#include "../../../common/nujel/lib/api.h"

#include <stdio.h>

#define ID_FIRE_UPDATE_RATE (0x10)

void itemDropUpdateMsg(u8 c,uint i){
	if(i >= itemDropCount)         {return;}
	if(itemDropList[i].ent == NULL){return;}
	msgItemDropUpdate(c,itemDropList[i].ent->pos,itemDropList[i].ent->vel,&itemDropList[i].itm,i,itemDropCount,itemDropList[i].player);
}

uint itemDropUpdatePlayer(uint c, uint offset){
	const item iZero = {0,0};
	const uint max = MIN(offset+clients[c].itemDropUpdateWindowSize,itemDropCount);
	if(itemDropCount == 0){msgItemDropUpdate(c,vecZero(),vecZero(),&iZero,0,0,-1);}
	for(uint i=0;i<clients[c].itemDropPriorityQueueLen;i++){
		itemDropUpdateMsg(c,clients[c].itemDropPriorityQueue[i]);
	}
	clients[c].itemDropPriorityQueueLen = 0;
	for(uint i=offset;i<max;i++){
		itemDropUpdateMsg(c,i);
	}
	offset += clients[c].itemDropUpdateWindowSize;
	if(offset >= itemDropCount){offset=0;}
	if(getClientLatency(c) < 20){
		clients[c].itemDropUpdateWindowSize += 2;
	}else{
		clients[c].itemDropUpdateWindowSize /= 2;
	}
	clients[c].itemDropUpdateWindowSize = MAX(2,MIN(16,clients[c].itemDropUpdateWindowSize));
	return offset;
}

itemDrop *itemDropNew(){
	if(itemDropFirstFree >= 0){
		const uint i = itemDropFirstFree;
		itemDropFirstFree = itemDropList[itemDropFirstFree].nextFree;
		addPriorityItemDrop(i);
		return &itemDropList[i];
	}
	if((itemDropCount) >= countof(itemDropList)){
		itemDropDel(rngValM(1<<14));
		return itemDropNew();
	}
	addPriorityItemDrop(itemDropCount);
	itemDropList[itemDropCount].nextFree = -1;
	return &itemDropList[itemDropCount++];
}

void itemDropNewP(const vec pos,const item *itm, i16 IDPlayer){
	if(itm == NULL){return;}
	itemDrop *id = itemDropNew();
	if(id == NULL){return;}

	id->itm      = *itm;
	id->ent      = entityNew(pos,vecZero(),itemGetWeight(&id->itm));
	id->player   = IDPlayer;
	id->lastFire =  0;
	id->fireDmg  =  0;
	entityUpdateCurChungus(id->ent);
	itemDropUpdateFire(id - itemDropList);
}

void itemDropNewPacket(uint c, const packet *p){
	itemDrop *id = itemDropNew();
	if(id == NULL){return;}

	id->itm.ID     = p->v.u16[12];
	id->itm.amount = p->v.i16[13];
	id->ent        = entityNew(vecNewP(&p->v.f[0]),vecZero(),itemGetWeight(&id->itm));
	id->ent->vel   = vecNewP(&p->v.f[3]);
	id->player     = c;
	id->lastFire   = 0;
	id->fireDmg    = 0;
	entityUpdateCurChungus(id->ent);
}

void itemDropPickupP(uint c, const packet *p){
	uint i = p->v.u16[0];
	if(i >= itemDropCount)      {return;}
	if(c >= clientCount)        {return;}
	if(clients[c].c == NULL)    {return;}
	if(itemDropList[i].ent == NULL){return;}
	const vec dist = vecSub(clients[c].c->pos,itemDropList[i].ent->pos);
	const float dd = vecDot(dist,dist);
	if(dd > 32.f * 32.f)     {return;}
	msgPickupItem(c,itemDropList[i].itm);
	itemDropDel(i);
	addPriorityItemDrop(i);
}

void itemDropBounceP(uint c, const packet *p){
	uint i = p->v.u16[0];
	if(i >= itemDropCount)         {return;}
	if(c >= clientCount)           {return;}
	if(clients[c].c == NULL)       {return;}
	if(itemDropList[i].ent == NULL){return;}
	const vec dist = vecSub(clients[c].c->pos,itemDropList[i].ent->pos);
	const float dd = vecDot(dist,dist);
	if(dd > 32.f * 32.f)           {return;}
	itemDropList[i].ent->vel = vecMulS(itemDropList[i].ent->vel, -0.8f);
	addPriorityItemDrop(i);
}

static int itemDropCheckSubmersion(uint i){
	entity *e = itemDropList[i].ent;
	if(e == NULL){return 0;}
	if(worldGetB(e->pos.x,e->pos.y,e->pos.z) == 0){return 0;}

	for(int a=0;a<6;a++){
		int xo=0,yo=0,zo=0;
		switch(a){
			case 0: yo= 1; break;
			case 1: xo= 1; break;
			case 2: xo=-1; break;
			case 3: zo= 1; break;
			case 4: zo=-1; break;
			case 5: yo=-1; break;
		}
		if(worldGetB(e->pos.x+xo,e->pos.y+yo,e->pos.z+zo) == 0){
			e->pos = vecAdd(e->pos,vecNew(xo,yo,zo));
			addPriorityItemDrop(i);
			return 0;
		}
	}

	for(int x=-1;x<2;x+=2){
	for(int y=1;y>-2;y-=2){
	for(int z=-1;z<2;z+=2){
		if(worldGetB(e->pos.x+x,e->pos.y+y,e->pos.z+z) == 0){
			e->pos = vecAdd(e->pos,vecNew(x,y,z));
			addPriorityItemDrop(i);
			return 0;
		}
	}
	}
	}
	return 1;
}

static int itemDropCheckCollation(uint ai){
	if(itemDropList[ai].ent == NULL){return 0;}
	const vec a = itemDropList[ai].ent->pos;

	for(int i=0;i<4;i++){
		const uint  bi = rngValM(itemDropCount);
		if(bi == ai)                                          {continue;}
		if(itemDropList[ai].itm.ID != itemDropList[bi].itm.ID){continue;}
		if(itemDropList[bi].ent == NULL)                      {continue;}
		const vec    b = itemDropList[bi].ent->pos;
		const vec    d = vecSub(b,a);
		const float di = vecDot(d,d);
		if(di < 2.f){
			itemDropList[bi].itm.amount += itemDropList[ai].itm.amount;
			itemDropList[bi].fireDmg    += itemDropList[ai].fireDmg;
			itemDropList[bi].ent->vel    = vecAdd(itemDropList[bi].ent->vel,itemDropList[ai].ent->vel);
			itemDropList[bi].ent->pos    = vecMulS(vecAdd(itemDropList[bi].ent->pos,itemDropList[ai].ent->pos),0.5f);
			entityUpdateCurChungus(itemDropList[bi].ent);
			return 1;
		}
	}
	return 0;
}

void itemDropUpdateAll(){
	static uint calls = 0;
	PROFILE_START();

	for(uint i=itemDropCount-1;i<itemDropCount;i--){
		entity *e = itemDropList[i].ent;
		if(e == NULL){continue;}
		const int ret = entityUpdate(e);
		(void)ret;

		if((ret < 0) || itemDropCheckCollation(i) || itemDropCheckSubmersion(i)){
			itemDropDel(i);
			addPriorityItemDrop(i);
			continue;
		}

		const uint chance = itemGetIDChance(&itemDropList[i].itm);
		if((chance > 0) && (rngValA(chance) == 0)){
			lVal *r = lispCallFuncVII("item-drop-cb",itemDropList[i].ent->pos, itemDropList[i].itm.ID, itemDropList[i].itm.amount);
			if(r != NULL){ itemDropList[i].itm.amount += r->vInt; }
		}
	}

	for(uint i=itemDropCount-(1 + (calls&(ID_FIRE_UPDATE_RATE-1)));i<itemDropCount;i-=ID_FIRE_UPDATE_RATE){
		itemDropUpdateFire(i);
	}
	/*
	for(uint i=itemDropCount-1;i<itemDropCount;i--){
		itemDropUpdateFire(i);
	}
	*/
	calls++;

	PROFILE_STOP();
}

void itemDropUpdateFire(uint i){
	itemDrop *id = &itemDropList[i];
	entity *e    = id->ent;
	if(id->itm.amount <= 0){return;}
	if(e == NULL)   {return;}
	if(e->pos.y < 0){return;}
	const uint cx = e->pos.x;
	const uint cy = e->pos.y;
	const uint cz = e->pos.z;
	fire *f = &fireList[id->lastFire];
	if((id->lastFire >= fireCount) || (f->x != cx) || (f->y != cy) || (f->z != cz)){
		f = fireGetAtPos(cx,cy,cz);
		id->lastFire = (u16)(f - fireList);
	}
	if((f != NULL) && (f->x == cx) && (f->y == cy) && (f->z == cz)){
		const int dmg = MIN(f->oxygen,itemGetFireDamage(&id->itm) * id->itm.amount);
		id->fireDmg += dmg;
		f->strength += dmg - id->itm.amount;
		f->oxygen   -= dmg;
	}else if(id->fireDmg > 0){
		const int dmg = MIN(id->fireDmg,itemGetFireDamage(&id->itm) * id->itm.amount);
		id->fireDmg -= dmg;
		fireNew(cx,cy,cz,dmg);
	}

	int maxhp = itemGetFireHealth(&id->itm) * id->itm.amount;
	if(id->fireDmg >= maxhp){
		if(id->ent != NULL){
			lispCallFuncVII("item-burn-up",id->ent->pos, id->itm.ID , id->itm.amount);
		}
		itemDropDel(i);
		addPriorityItemDrop(i);
		id->fireDmg = 0;
	}
}

uint itemDropGetActive(){
	return itemDropCount;
}

uint itemDropGetSlow(){
	uint ret = 0;
	for(uint i=0;i<itemDropCount;i++){
		entity *e = itemDropList[i].ent;
		if(e == NULL){continue;}
		if(e->flags & ENTITY_SLOW_UPDATE){ret++;}
	}
	return ret;
}
