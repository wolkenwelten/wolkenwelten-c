#include "itemDrop.h"

#include "../game/entity.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../voxel/chungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>

itemDrop itemDrops[ITEM_DROPS_MAX];
uint     itemDropCount;

#define ITEM_DROPS_PER_UPDATE 16

static inline void itemDropUpdateMsg(uint c,uint i){
	if(i >= itemDropCount)       {return;}
	if(itemDrops[i].ent == NULL) {return;}
	msgItemDropUpdate(
		c,
		itemDrops[i].ent->pos,
		itemDrops[i].ent->vel,
		i,
		itemDropCount,
		itemDrops[i].itm.ID,
		itemDrops[i].itm.amount
	);
}

uint itemDropUpdatePlayer(uint c, uint offset){
	const uint max = MIN(offset+ITEM_DROPS_PER_UPDATE,itemDropCount);
	if(itemDropCount == 0){msgItemDropUpdate(c,vecZero(),vecZero(),0,0,0,0);}
	for(uint i=0;i<clients[c].itemDropPriorityQueueLen;i++){
		itemDropUpdateMsg(c,clients[c].itemDropPriorityQueue[i]);
	}
	clients[c].itemDropPriorityQueueLen = 0;
	for(uint i=offset;i<max;i++){
		itemDropUpdateMsg(c,i);
	}
	offset += ITEM_DROPS_PER_UPDATE;
	if(offset >= itemDropCount){offset=0;}
	return offset;
}

itemDrop *itemDropNew(){
	if((itemDropCount) >= (int)(sizeof(itemDrops) / sizeof(itemDrop) - 1)){return NULL;}
	addPriorityItemDrop(itemDropCount);
	return &itemDrops[itemDropCount++];
}

void itemDropNewP(const vec pos,const item *itm){
	if(itm == NULL){return;}
	itemDrop *id = itemDropNew();
	if(id == NULL){return;}

	id->itm = *itm;
	id->ent = entityNew(pos,vecZero());
	id->player = -1;
}

void itemDropNewC(uint c, const packet *p){
	itemDrop *id = itemDropNew();
	if(id == NULL){return;}

	id->ent        = entityNew(vecNewP(&p->val.f[0]),vecZero());
	id->ent->vel   = vecNewP(&p->val.f[3]);
	id->itm.ID     = p->val.i[6];
	id->itm.amount = p->val.i[7];
	id->player     = c;
}

void itemDropDel(uint d){
	if(d >= itemDropCount) {return;}

	entityFree(itemDrops[d].ent);
	itemDrops[d].ent = NULL;
	itemDrops[d]     = itemDrops[--itemDropCount];
}
void itemDropDelChungus(const chungus *c){
	if(c == NULL){return;}
	for(uint i=itemDropCount-1;i<itemDropCount;i--){
		if(itemDrops[i].ent->curChungus != c){continue;}
		itemDropDel(i);
	}
}

static bool itemDropCheckPickup(uint d){
	for(uint i=0;i<clientCount;++i){
		if(clients[i].c == NULL){continue;}
		const vec dist = vecSub(clients[i].c->pos,itemDrops[d].ent->pos);
		if((uint)itemDrops[d].player == i){
			if(vecDot(dist,dist) > (2.f*2.f)){ itemDrops[d].player = -1; }
		}else if(vecDot(dist,dist) < (1.5f*1.5f)){
			msgPickupItem(i,itemDrops[d].itm.ID,itemDrops[d].itm.amount);
			addPriorityItemDrop(d);
			return true;
		}
	}
	return false;
}

static int itemDropCheckSubmersion(uint i){
	entity *e = itemDrops[i].ent;
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
	if(itemDrops[ai].ent == NULL){return 0;}
	const vec a = itemDrops[ai].ent->pos;

	for(int i=0;i<4;i++){
		const uint  bi = rngValM(itemDropCount);
		if(bi == ai)                                     {continue;}
		if(itemDrops[ai].itm.ID != itemDrops[bi].itm.ID) {continue;}
		if(itemDrops[bi].ent == NULL)                    {continue;}
		const vec    b = itemDrops[bi].ent->pos;
		const vec    d = vecSub(b,a);
		const float di = vecDot(d,d);
		if(di < 0.75f){
			itemDrops[bi].itm.amount += itemDrops[ai].itm.amount;
			itemDrops[bi].ent->vel = vecAdd(itemDrops[bi].ent->vel,itemDrops[ai].ent->vel);
			itemDrops[bi].ent->pos = vecMulS(vecAdd(itemDrops[bi].ent->pos,itemDrops[ai].ent->pos),0.5f);
			addPriorityItemDrop(bi);
			return 1;
		}
	}
	return 0;
}

void itemDropUpdate(){
	const u64 mask = ~((u64)1 << 31);

	for(uint i=itemDropCount-1;i<itemDropCount;i--){
		int oldp,newp;
		entity *e     = itemDrops[i].ent;
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
		if(itemDropCheckCollation(i) || itemDropCheckSubmersion(i) || itemDropCheckPickup(i) || (e->pos.y < -256)){
			itemDropDel(i);
			addPriorityItemDrop(i);
			continue;
		}
	}
}

void itemDropIntro(uint c){
	for(uint i=0;i<itemDropCount;i++){
		msgItemDropUpdate(
			c,
			itemDrops[i].ent->pos,
			itemDrops[i].ent->vel,
			i,
			itemDropCount,
			itemDrops[i].itm.ID,
			itemDrops[i].itm.amount
		);
	}
}

uint itemDropGetActive(){
	return itemDropCount;
}
