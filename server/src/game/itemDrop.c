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
int      itemDropCount = 0;

#define ITEM_DROPS_PER_UPDATE 16

inline void itemDropUpdateMsg(int c,unsigned int i){
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

unsigned int itemDropUpdatePlayer(int c, unsigned int offset){
	const int max = MIN((int)(offset+ITEM_DROPS_PER_UPDATE),itemDropCount);
	if(itemDropCount == 0){msgItemDropUpdate(c,vecZero(),vecZero(),0,0,0,0);}
	for(unsigned int i=0;i<clients[c].itemDropPriorityQueueLen;i++){
		itemDropUpdateMsg(c,clients[c].itemDropPriorityQueue[i]);
	}
	clients[c].itemDropPriorityQueueLen = 0;
	for(int i=offset;i<max;i++){
		itemDropUpdateMsg(c,i);
	}
	offset += ITEM_DROPS_PER_UPDATE;
	if((int)offset >= itemDropCount){offset=0;}
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

void itemDropDel(int d){
	if(d < 0)              {return;}
	if(d >= itemDropCount) {return;}

	entityFree(itemDrops[d].ent);
	itemDrops[d].ent = NULL;
	itemDrops[d]     = itemDrops[--itemDropCount];
}

bool itemDropCheckPickup(int d){
	for(int i=0;i<clientCount;++i){
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
	const uint64_t mask = ~((uint64_t)1 << 31);

	for(int i=0;i<itemDropCount;i++){
		int oldp,newp;
		entity *e = itemDrops[i].ent;
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

void itemDropIntro(int c){
	for(int i=0;i<itemDropCount;i++){
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

void *itemDropSave(itemDrop *i, void *buf){
	uint8_t  *b = (uint8_t *) buf;
	uint16_t *s = (uint16_t *)buf;
	float    *f = (float *)   buf;

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

void *itemDropLoad(void *buf){
	uint8_t  *b = (uint8_t *) buf;
	uint16_t *s = (uint16_t *)buf;
	float    *f = (float *)   buf;

	itemDrop *id = itemDropNew();
	if(id == NULL){return b+32;}
	id->itm.ID     = s[1];
	id->itm.amount = s[2];

	id->ent = entityNew(vecNewP(&f[2]),vecZero());
	if(id->ent == NULL){return b+32;}
	id->ent->vel = vecNewP(&f[5]);

	return b+32;
}

void *itemDropSaveChungus(chungus *c,void *buf){
	if(c == NULL){return buf;}
	for(int i=0;i<itemDropCount;i++){
		if(itemDrops[i].ent->curChungus != c){continue;}
		buf = itemDropSave(&itemDrops[i],buf);
	}
	return buf;
}

void itemDropDelChungus(chungus *c){
	if(c == NULL){return;}
	for(int i=itemDropCount-1;i>=0;i--){
		if(itemDrops[i].ent->curChungus != c){continue;}
		itemDropDel(i);
	}
}
