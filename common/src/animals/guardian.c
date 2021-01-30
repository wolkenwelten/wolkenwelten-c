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

#include "guardian.h"

#include "../game/animal.h"
#include "../game/character.h"
#include "../game/entity.h"
#include "../game/item.h"
#include "../game/projectile.h"
#include "../network/messages.h"
#include "../misc/misc.h"
#include "../mods/api_v1.h"

#include <math.h>

static void animalAggresive(animal *e){
	character *cChar;
	float dist = animalClosestPlayer(e,&cChar);
	uint los   = 0;
	if(cChar != NULL){
		los = lineOfSightBlockCount(vecAdd(e->pos,vecNew(0.f,.5f,0.f)),cChar->pos,2);
	}

	if(e->state == ANIMAL_S_FIGHT){
		if((cChar == NULL) || (dist > 128.f) || los){
			e->state   =  ANIMAL_S_LOITER;
		}else{
			vec caNorm   = vecNorm(vecSub(cChar->pos, e->pos));
			vec caRot    = vecVecToDeg(caNorm);

			e->rot.yaw   = -caRot.yaw;
			e->rot.pitch = -caRot.pitch;
			if(e->rot.pitch >  90.f){e->rot.pitch -= 180.f;}
			if(e->rot.pitch < -90.f){e->rot.pitch += 180.f;}
			if(caRot.pitch >  90.f){caRot.pitch -= 180.f;}
			if(caRot.pitch < -90.f){caRot.pitch += 180.f;}
			e->grot      = e->rot;
			e->gvel.x    = 0;
			e->gvel.z    = 0;

			projectileNew(vecSub(e->pos,caNorm), vecNew(0.f,-90.f,0.f), characterGetBeing(cChar), animalGetBeing(e), 2, 0.2f);
			if(e->temp == 0){
				e->state = ANIMAL_S_FLEE;
				e->grot  = vecZero();
				e->temp  = 50;
			}
		}
	}else if(e->state == ANIMAL_S_FLEE){
		if(e->temp == 0){
			e->state = ANIMAL_S_LOITER;
		}else if(cChar != NULL){
			vec caNorm  = vecNorm(vecNew(e->pos.x - cChar->pos.x,0.f, e->pos.z - cChar->pos.z));
			vec caVel   = vecMulS(caNorm,0.03f);
			vec caRot   = vecVecToDeg(caNorm);

			e->gvel.x   = caVel.x;
			e->gvel.z   = caVel.z;
			e->rot.yaw  = -caRot.yaw;
			e->grot.yaw = e->rot.yaw;
		}
	}else{
		if((e->temp == 0) && (cChar != NULL) && (dist < 96.f) && !los){
			if(rngValA(31) == 0){
				vec caNorm   = vecNorm(vecSub(cChar->pos, e->pos));
				projectileNew(vecSub(e->pos,caNorm), vecNew(0.f,-90.f,0.f), characterGetBeing(cChar), animalGetBeing(e), 3, 0.4f);
				e->temp  = 50;
			}else{
				e->state = ANIMAL_S_FIGHT;
				e->temp  = 2;
			}
		}
	}
	if(e->temp > 0){e->temp = MIN(e->temp-1,64);}
}

static void animalSLoiter(animal *e){
	if(rngValA( 7) == 0){
		e->grot.yaw = e->rot.yaw + ((rngValf()*2.f)-1.f)*4.f;
	}
	if(rngValA(15) == 0){
		e->gvel = vecZero();
	}
	if(rngValA(31) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*10.f;
	}
	if(rngValA(63) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValA(31) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}
}

void animalThinkGuardian(animal *e){
	animalCheckSuffocation(e);
	animalAggresive(e);

	switch(e->state){
	default:
		break;
	case ANIMAL_S_LOITER:
		animalSLoiter(e);
		break;
	}
}

void animalRDieGuardian(animal *e){
	item drop = itemNew(I_Crystalbullet,rngValMM(20,40));
	itemDropNewP(e->pos,&drop);
}

void animalRBurnGuardian(animal *e){
	(void)e;
}
