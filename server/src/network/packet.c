/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
