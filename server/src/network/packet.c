#include "packet.h"

#include "server.h"
#include <string.h>

void packetReadyS(packetSmall  *p, uint16_t ptype){
	p->ptype = (ptype & (~0xC000));
}

void packetReadyM(packetMedium *p, uint16_t ptype){
	p->ptype = (ptype & (~0xC000)) | (0x4000);
}

void packetReadyL(packetLarge  *p, uint16_t ptype){
	p->ptype = (ptype & (~0xC000)) | (0x8000);
}

void packetReadyH(packetHuge   *p, uint16_t ptype){
	p->ptype = ptype | 0xC000;
}

void packetQueueS(packetSmall  *p, uint16_t ptype, int c){
	packetReadyS(p,ptype);
	if(c >= 0){
		sendToClient(c,p,sizeof(packetSmall));
	}else{
		sendToAll(p,sizeof(packetSmall));
	}
}

void packetQueueM(packetMedium *p, uint16_t ptype, int c){
	packetReadyM(p,ptype);
	if(c >= 0){
		sendToClient(c,p,sizeof(packetMedium));
	}else{
		sendToAll(p,sizeof(packetMedium));
	}
}

void packetQueueL(packetLarge  *p, uint16_t ptype, int c){
	packetReadyL(p,ptype);
	if(c >= 0){
		sendToClient(c,p,sizeof(packetLarge));
	}else{
		sendToAll(p,sizeof(packetLarge));
	}
}

void packetQueueH(packetHuge   *p, uint16_t ptype, int c){
	packetReadyH(p,ptype);
	if(c >= 0){
		sendToClient(c,p,sizeof(packetHuge));
	}else{
		sendToAll(p,sizeof(packetHuge));
	}
}

unsigned int packetLen(const void *p){
	if(p == NULL){return -1;}
	const packetSmall *ps = (const packetSmall *)p;
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

