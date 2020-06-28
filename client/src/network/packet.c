#include "packet.h"
#include "../network/client.h"
#include <string.h>


void packetQueueS(packetSmall  *p, uint16_t ptype){
	p->ptype = (ptype & (~0xC000));
	queueToServer(p,sizeof(packetSmall));
}

void packetQueueM(packetMedium *p, uint16_t ptype){
	p->ptype = (ptype & (~0xC000)) | (0x4000);
	queueToServer(p,sizeof(packetMedium));
}

void packetQueueL(packetLarge  *p, uint16_t ptype){
	p->ptype = (ptype & (~0xC000)) | (0x8000);
	queueToServer(p,sizeof(packetLarge));
}

void packetQueueH(packetHuge   *p, uint16_t ptype){
	p->ptype = ptype | 0xC000;
	queueToServer(p,sizeof(packetHuge));
}

int packetLen(void *p){
	if(p == NULL){return -1;}
	packetSmall *ps = (packetSmall *)p;
	switch(ps->ptype >> 14){
		case 0:
			return sizeof(packetSmall);

		case 1:
			return sizeof(packetMedium);

		case 2:
			return sizeof(packetLarge);

		case 3:
			return sizeof(packetHuge);

		default:
			return -1;
	}
}

void packetQueueChunk(chunk *chnk){
	static packetHuge p;
	memcpy(&p.val.c,&chnk->data,4096);
	p.val.u[1024] = chnk->x;
	p.val.u[1025] = chnk->y;
	p.val.u[1026] = chnk->z;
	packetQueueH(&p,1);
}

