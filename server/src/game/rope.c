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
#include "rope.h"

#include "../network/server.h"
#include "../../../common/src/network/messages.h"

int ropeNewID(){
	for(uint i=256;i<512;i++){
		if(ropeList[i].a == 0){return i;}
		if(ropeList[i].b == 0){return i;}
	}
	return -1;
}

void ropeDelBeing(const being t){
	for(uint i = 256;i<512;i++){
		if((ropeList[i].a == t) || (ropeList[i].b == t)){
			ropeList[i].flags = 0;
			ropeList[i].a     = 0;
			ropeList[i].b     = 0;
		}
	}
}
