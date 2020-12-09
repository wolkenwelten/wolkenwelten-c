#include "../network/client.h"

void packetQueue(packet *p, u8 ptype, uint len, int c){
	(void)c;
	packetSet(p,ptype,len);
	queueToServer(p,len+4);
}

void packetQueueExcept(packet *p, u8 ptype, uint len, int c){
	(void)c;
	packetSet(p,ptype,len);
	queueToServer(p,len+4);
}

void packetQueueToServer(packet *p, u8 ptype, uint len){
	packetSet(p,ptype,len);
	queueToServer(p,len+4);
}
