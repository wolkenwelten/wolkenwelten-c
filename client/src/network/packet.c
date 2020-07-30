#include "../network/client.h"

void packetQueue(packet *p, uint8_t ptype, unsigned int len, int c){
	(void)c;
	p->typesize = (len << 10) | (ptype);
	queueToServer(p,len+4);
}

void packetQueueExcept(packet *p, uint8_t ptype, unsigned int len, int c){
	(void)c;
	p->typesize = (len << 10) | (ptype);
	queueToServer(p,len+4);
}

void packetQueueToServer(packet *p, uint8_t ptype, unsigned int len){
	p->typesize = (len << 10) | (ptype);
	queueToServer(p,len+4);
}
