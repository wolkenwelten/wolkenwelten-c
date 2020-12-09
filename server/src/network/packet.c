#include "../../../common/src/network/packet.h"

#include "server.h"

#include <stdio.h>

void packetQueue(packet *p, u8 ptype, uint len, int c){
	packetSet(p,ptype,len);
	if(c >= 0){
		sendToClient(c,p,len+4);
	}else{
		sendToAll(p,len+4);
	}
}

void packetQueueExcept(packet *p, u8 ptype, uint len, int c){
	packetSet(p,ptype,len);
	sendToAllExcept(c,p,len+4);
}

void packetQueueToServer(packet *p, u8 ptype, uint len){
	(void)p;
	(void)ptype;
	(void)len;
	fprintf(stderr,"Called a function intended for client use only from the Server\n");
}
