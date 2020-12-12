#include "itemDrop.h"

#include "../game/entity.h"
#include "../game/fire.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/mods/mods.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>

static inline void itemDropEmptyMsg(uint c, uint i){
	item itm = itemEmpty();
	msgItemDropUpdate(
		c,
		vecNOne(),
		vecZero(),
		&itm,
		i,
		itemDropCount
	);
}

static inline void itemDropUpdateMsg(uint c,uint i){
	if(i >= itemDropCount)         {return;}
	if(itemDropList[i].ent == NULL){return;}
	msgItemDropUpdate(
		c,
		itemDropList[i].ent->pos,
		itemDropList[i].ent->vel,
		&itemDropList[i].itm,
		i,
		itemDropCount
	);
}

uint itemDropUpdatePlayer(uint c, uint offset){
	const item iZero = {0,0};
	const uint max = MIN(offset+clients[c].itemDropUpdateWindowSize,itemDropCount);
	if(itemDropCount == 0){msgItemDropUpdate(c,vecZero(),vecZero(),&iZero,0,0);}
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

void itemDropDel(uint d){
	if(d >= itemDropCount) {return;}

	entityFree(itemDropList[d].ent);
	itemDropList[d].ent = NULL;
	itemDropList[d].itm = itemEmpty();
	itemDropList[d].nextFree = itemDropFirstFree;
	itemDropFirstFree = d;
	itemDropEmptyMsg(-1,d);
}

void itemDropNewP(const vec pos,const item *itm){
        if(itm == NULL){return;}
        itemDrop *id = itemDropNew();
        if(id == NULL){return;}

        id->itm      = *itm;
        id->ent      = entityNew(pos,vecZero());
        id->player   = -1;
        id->lastFire =  0;
        id->fireDmg  =  0;
        entityUpdateCurChungus(id->ent);
        itemDropUpdateFire(id - itemDropList);
}

void itemDropNewPacket(uint c, const packet *p){
	itemDrop *id = itemDropNew();
	if(id == NULL){return;}

	id->ent        = entityNew(vecNewP(&p->v.f[0]),vecZero());
	id->ent->vel   = vecNewP(&p->v.f[3]);
	id->itm.ID     = p->v.u16[12];
	id->itm.amount = p->v.i16[13];
	id->player     = c;
	id->lastFire   = 0;
	id->fireDmg    = 0;
	entityUpdateCurChungus(id->ent);
}

void itemDropDelChungus(const chungus *c){
	if(c == NULL){return;}
	for(uint i=itemDropCount-1;i<itemDropCount;i--){
		if(itemIsEmpty(&itemDropList[i].itm))   {continue;}
		if(itemDropList[i].ent == NULL)         {continue;}
		const vec *p = &itemDropList[i].ent->pos;
		if(((uint)p->x >> 8) != c->x){continue;}
		if(((uint)p->y >> 8) != c->y){continue;}
		if(((uint)p->z >> 8) != c->z){continue;}
		itemDropDel(i);
	}
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

	for(int x=-1;x<2;x++){
		if(x == 0){continue;}
		for(int y=1;y>-2;y--){
			if(y == 0){continue;}
			for(int z=-1;z<2;z++){
				if(z == 0){continue;}
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
		if(di < 0.75f){
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
	const u64 mask = ~((u64)1 << 31);

	for(uint i=itemDropCount-1;i<itemDropCount;i--){
		int oldp,newp;
		entity *e     = itemDropList[i].ent;
		if(e == NULL){continue;}
		chungus *oldc = e->curChungus;

		oldp = ((int)e->pos.x&0xFF)|(((int)e->pos.y&0xFF)<<8)|(((int)e->pos.z&0xFF)<<16);
		entityUpdate(e);
		newp = ((int)e->pos.x&0xFF)|(((int)e->pos.y&0xFF)<<8)|(((int)e->pos.z&0xFF)<<16);

		if(oldc != e->curChungus){
			if(oldc != NULL){oldc->clientsUpdated &= mask;}
			oldp = 1<<30;
		}
		if((oldp != newp) && (e->curChungus != NULL)){
			e->curChungus->clientsUpdated &= mask;
		}
		itemDropList[i].itm.amount += itemDropCallbackDispatch(&itemDropList[i].itm, e->pos.x, e->pos.y,e->pos.z);
		if((itemDropList[i].itm.amount < 0) || itemDropCheckCollation(i) || itemDropCheckSubmersion(i) || (e->pos.y < -256)){
			itemDropDel(i);
			addPriorityItemDrop(i);
			continue;
		}
	}
}

void itemDropUpdateFire(uint i){
	itemDrop *id = &itemDropList[i];
	entity *e    = id->ent;
	if(e == NULL){return;}
	const u16 cx = e->pos.x;
	const u16 cy = e->pos.y;
	const u16 cz = e->pos.z;
	fire *f = &fireList[id->lastFire];
	if((id->lastFire >= fireCount) || (f->x != cx) || (f->y != cy) || (f->z != cz)){
		f = fireGetAtPos(cx,cy,cz);
		id->lastFire = (u16)(f - fireList);
	}
	if((f != NULL) && (f->x == cx) && (f->y == cy) && (f->z == cz)){
		const int dmg = MIN(f->oxygen,getFireDmgDispatch(id) * id->itm.amount);
		id->fireDmg += dmg;
		f->strength += dmg - id->itm.amount;
		f->oxygen   -= dmg;
	}else if(id->fireDmg > 0){
		const int dmg = MIN(id->fireDmg,getFireDmgDispatch(id) * id->itm.amount);
		fireNew(cx,cy,cz,dmg);
	}

	int maxhp = getFireHealthDispatch(id) * id->itm.amount;
	if(id->fireDmg >= maxhp){
		if(!itemDropBurnUpDispatch(id)){
			itemDropDel(i);
		}
		addPriorityItemDrop(i);
		id->fireDmg = 0;
	}
}

void itemDropUpdateFireAll(){
	static uint calls = 0;
	for(uint i=itemDropCount-(1+(calls&0x1F));i<itemDropCount;i-=0x20){
		itemDropUpdateFire(i);
	}
	calls++;
}

void itemDropIntroChungus(uint c,const chungus *chng){
	for(uint i=0;i<itemDropCount;i++){
		if(itemDropList[i].ent == NULL)            {continue;}
		if(itemDropList[i].ent->curChungus != chng){continue;}
		msgItemDropUpdate(c,itemDropList[i].ent->pos,itemDropList[i].ent->vel,&itemDropList[i].itm,i,itemDropCount);
	}
}

uint itemDropGetActive(){
	return itemDropCount;
}
