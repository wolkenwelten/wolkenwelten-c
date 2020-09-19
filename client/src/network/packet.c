#include "../network/client.h"

void packetQueue(packet *p, u8 ptype, uint len, int c){
	(void)c;
	p->typesize = (len << 10) | (ptype);
	queueToServer(p,len+4);
}

void packetQueueExcept(packet *p, u8 ptype, uint len, int c){
	(void)c;
	p->typesize = (len << 10) | (ptype);
	queueToServer(p,len+4);
}

void packetQueueToServer(packet *p, u8 ptype, uint len){
	p->typesize = (len << 10) | (ptype);
	queueToServer(p,len+4);
}
