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
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

inline void itemDropUpdateMsg(int c, int i){
	msgItemDropUpdate(
		c,
		itemDrops[i].ent->x,
		itemDrops[i].ent->y,
		itemDrops[i].ent->z,
		itemDrops[i].ent->vx,
		itemDrops[i].ent->vy,
		itemDrops[i].ent->vz,
		i,
		itemDropCount,
		itemDrops[i].itm.ID,
		itemDrops[i].itm.amount
	);
}

unsigned int itemDropUpdatePlayer(int c, unsigned int offset){
	const int max = MIN((int)(offset+ITEM_DROPS_PER_UPDATE),itemDropCount);
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

void itemDropNewP(float x, float y, float z,const item *itm){
	if(itm == NULL){return;}
	itemDrop *id = itemDropNew();
	if(id == NULL){return;}

	id->itm     = *itm;
	id->ent     = entityNew(x,y,z,0.f,0.f,0.f);
}

void itemDropNewC(const packet *p){
	itemDrop *id = itemDropNew();
	if(id == NULL){return;}

	id->ent        = entityNew(0.f,0.f,0.f,0.f,0.f,0.f);
	id->ent->x     = p->val.f[0];
	id->ent->y     = p->val.f[1];
	id->ent->z     = p->val.f[2];
	id->ent->vx    = p->val.f[3];
	id->ent->vy    = p->val.f[4];
	id->ent->vz    = p->val.f[5];
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
		float dx = clients[i].c->x - itemDrops[d].ent->x;
		float dy = clients[i].c->y - itemDrops[d].ent->y;
		float dz = clients[i].c->z - itemDrops[d].ent->z;
		if(((dx*dx)+(dy*dy)+(dz*dz)) < (1.5f*1.5f)){
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
		
		oldp = ((int)e->x&0xFF)|(((int)e->y&0xFF)<<8)|(((int)e->z&0xFF)<<16);
		entityUpdate(e);
		newp = ((int)e->x&0xFF)|(((int)e->y&0xFF)<<8)|(((int)e->z&0xFF)<<16);
		
		if(oldc != e->curChungus){
			if(oldc != NULL){oldc->clientsUpdated &= mask;}
			oldp = 1<<30;
		}
		if((oldp != newp) && (e->curChungus != NULL)){
			e->curChungus->clientsUpdated &= mask;
		}
		if(itemDropCheckPickup(i) || (e->y < -256)){
			itemDropDel(i--);
			continue;
		}
	}
}

void itemDropIntro(int c){
	for(int i=0;i<itemDropCount;i++){
		msgItemDropUpdate(
			c,
			itemDrops[i].ent->x,
			itemDrops[i].ent->y,
			itemDrops[i].ent->z,
			itemDrops[i].ent->vx,
			itemDrops[i].ent->vy,
			itemDrops[i].ent->vz,
			i,
			itemDropCount,
			itemDrops[i].itm.ID,
			itemDrops[i].itm.amount
		);
	}
}

uint8_t *itemDropSave(itemDrop *i, uint8_t *b){
	uint16_t *s = (uint16_t *)b;
	float    *f = (float *)   b;

	if(i      == NULL){return b;}
	if(i->ent == NULL){return b;}

	b[0] = 0x02;
	b[1] = 0;

	s[1] = i->itm.ID;
	s[2] = i->itm.amount;
	s[3] = 0;
	
	f[2] = i->ent->x;
	f[3] = i->ent->y;
	f[4] = i->ent->z;
	f[5] = i->ent->vx;
	f[6] = i->ent->vy;
	f[7] = i->ent->vz;

	return b+32;
}

uint8_t *itemDropLoad(uint8_t *b){
	uint16_t *s = (uint16_t *)b;
	float    *f = (float *)   b;

	itemDrop *id = itemDropNew();
	if(id == NULL){return b+32;}
	id->itm.ID     = s[1];
	id->itm.amount = s[2];

	id->ent = entityNew(f[2],f[3],f[4],0.f,0.f,0.f);
	if(id->ent == NULL){return b+32;}
	id->ent->vx = f[5];
	id->ent->vy = f[6];
	id->ent->vz = f[7];

	return b+32;
}

uint8_t *itemDropSaveChungus(chungus *c,uint8_t *b){
	if(c == NULL){return b;}	
	for(int i=0;i<itemDropCount;i++){
		if(itemDrops[i].ent->curChungus != c){continue;}
		b = itemDropSave(&itemDrops[i],b);
	}
	return b;
}

void itemDropDelChungus(chungus *c){
	if(c == NULL){return;}
	for(int i=itemDropCount-1;i>=0;i--){
		if(itemDrops[i].ent->curChungus != c){continue;}
		itemDropDel(i);
	}
}
