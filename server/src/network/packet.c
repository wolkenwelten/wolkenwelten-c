#include "../../../common/src/packet.h"
#include "server.h"

#include <stdio.h>

void packetQueue(packet *p, uint8_t ptype, unsigned int len, int c){
	p->typesize = (len << 10) | (ptype);
	if(c >= 0){
		sendToClient(c,p,len+4);
	}else{
		sendToAll(p,len+4);
	}
}

void packetQueueExcept(packet *p, uint8_t ptype, unsigned int len, int c){
	p->typesize = (len << 10) | (ptype);
	sendToAllExcept(c,p,len+4);
}

void packetQueueToServer(packet *p, uint8_t ptype, unsigned int len){
	(void)p;
	(void)ptype;
	(void)len;
	fprintf(stderr,"Called a function intended for client use only from the Server\n");
}
