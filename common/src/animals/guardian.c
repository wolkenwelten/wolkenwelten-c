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

			if(--e->temp == 0){
				projectileNew(vecSub(e->pos,caNorm), vecNew(0.f,-90.f,0.f), characterGetBeing(cChar), animalGetBeing(e), 2);
				e->state = ANIMAL_S_FLEE;
				e->temp  = 30;
				e->grot  = vecZero();
			}
		}
	}else if(e->state == ANIMAL_S_FLEE){
		if((cChar == NULL) || (dist > 128.f) || (--e->temp == 0)){
			e->state = ANIMAL_S_LOITER;
		}else{
			vec caNorm  = vecNorm(vecNew(e->pos.x - cChar->pos.x,0.f, e->pos.z - cChar->pos.z));
			vec caVel   = vecMulS(caNorm,0.03f);
			vec caRot   = vecVecToDeg(caNorm);

			e->gvel.x   = caVel.x;
			e->gvel.z   = caVel.z;
			e->rot.yaw  = -caRot.yaw;
			e->grot.yaw = e->rot.yaw;
			if((dist < 3.f) && (rngValM(8)==0)){
				e->state = ANIMAL_S_FIGHT;
				e->flags |= ANIMAL_AGGRESIVE;
			}
		}
	}else{
		float fd = 96.f;
		if(e->state == ANIMAL_S_SLEEP){fd = 24.f;}
		if((cChar != NULL) && (dist < fd) && !los){
			e->state = ANIMAL_S_FIGHT;
			e->temp  = 30;
		}
	}
	e->temp = MIN(e->temp,64);
}

static void animalSLoiter(animal *e){
	if(rngValM( 8) == 0){
		e->grot.yaw = e->rot.yaw + ((rngValf()*2.f)-1.f)*4.f;
	}
	if(rngValM(16) == 0){
		e->gvel = vecZero();
	}
	if(rngValM(48) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*10.f;
	}
	if(rngValM(64) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValM(32) == 0){
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
	item drop = itemNew(I_Bullet,rngValMM(20,40));
	itemDropNewP(e->pos,&drop);
}
