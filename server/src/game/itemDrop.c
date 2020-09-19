#include "itemDrop.h"

#include "../main.h"
#include "../game/entity.h"
#include "../network/server.h"
#include "../voxel/chungus.h"
#include "../../../common/src/game/blockType.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct {
	entity    *ent;
	item       itm;
} itemDrop;

itemDrop itemDrops[1<<12];
uint     itemDropCount = 0;

#define ITEM_DROPS_PER_UPDATE 16

inline void itemDropUpdateMsg(uint c,uint i){
	if(i >= 4096)                {return;}
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

static itemDrop *itemDropNew(){
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
}

void itemDropNewC(const packet *p){
	itemDrop *id = itemDropNew();
	if(id == NULL){return;}

	id->ent        = entityNew(vecNewP(&p->val.f[0]),vecNewP(&p->val.f[3]));
	id->itm.ID     = p->val.i[6];
	id->itm.amount = p->val.i[7];
}

void itemDropDel(uint d){
	if(d >= itemDropCount) {return;}

	entityFree(itemDrops[d].ent);
	itemDrops[d].ent = NULL;
	itemDrops[d]     = itemDrops[--itemDropCount];
}

bool itemDropCheckPickup(uint d){
	for(uint i=0;i<clientCount;++i){
		if(clients[i].c == NULL){continue;}
		const vec dist = vecSub(clients[i].c->pos,itemDrops[d].ent->pos);
		if(vecMag(dist) < (1.5f*1.5f)){
			msgPickupItem(i,itemDrops[d].itm.ID,itemDrops[d].itm.amount);
			addPriorityItemDrop(d);
			return true;
		}
	}
	return false;
}

void itemDropUpdate(){
	const u64 mask = ~((u64)1 << 31);

	for(uint i=0;i<itemDropCount;i++){
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
		if(itemDropCheckPickup(i) || (e->pos.y < -256)){
			itemDropDel(i--);
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

void *itemDropSave(const itemDrop *i, void *buf){
	u8    *b = (u8 *)    buf;
	u16   *s = (u16 *)   buf;
	float *f = (float *) buf;

	if(i      == NULL){return b;}
	if(i->ent == NULL){return b;}

	b[0] = 0x02;
	b[1] = 0;

	s[1] = i->itm.ID;
	s[2] = i->itm.amount;
	s[3] = 0;

	f[2] = i->ent->pos.x;
	f[3] = i->ent->pos.y;
	f[4] = i->ent->pos.z;
	f[5] = i->ent->vel.x;
	f[6] = i->ent->vel.y;
	f[7] = i->ent->vel.z;

	return b+32;
}

const void *itemDropLoad(const void *buf){
	u8    *b = (u8 *)    buf;
	u16   *s = (u16 *)   buf;
	float *f = (float *) buf;

	itemDrop *id = itemDropNew();
	if(id == NULL){return b+32;}
	id->itm.ID     = s[1];
	id->itm.amount = s[2];

	id->ent = entityNew(vecNewP(&f[2]),vecZero());
	if(id->ent == NULL){return b+32;}
	id->ent->vel = vecNewP(&f[5]);

	return b+32;
}

void *itemDropSaveChungus(const chungus *c,void *buf){
	if(c == NULL){return buf;}
	for(uint i=0;i<itemDropCount;i++){
		if(itemDrops[i].ent->curChungus != c){continue;}
		buf = itemDropSave(&itemDrops[i],buf);
	}
	return buf;
}

void itemDropDelChungus(const chungus *c){
	if(c == NULL){return;}
	for(uint i=itemDropCount-1;i<itemDropCount;i--){
		if(itemDrops[i].ent->curChungus != c){continue;}
		itemDropDel(i);
	}
}
