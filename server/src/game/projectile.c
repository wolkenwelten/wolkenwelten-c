#include "projectile.h"

#include "../network/server.h"

void projectileSyncPlayer(u8 c){
	int count = 4;
	for(uint tries = 256;tries > 0;tries--){
		uint i = ++clients[c].projectileUpdateOffset;
		if(i >= countof(projectileList)){i = clients[c].projectileUpdateOffset = 0;}
		if(projectileList[i].style == 0){continue;}
		projectileSendUpdate(c,i);
		count--;
	}
}
